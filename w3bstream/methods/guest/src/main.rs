#![no_main]

use dewi_core::*;
use risc0_zkvm::guest::env;
//use serde_json::Value as JsonValue;
use std::collections::HashMap;

risc0_zkvm::guest::entry!(main);

fn main() {
    // Get the task information using the isolated get_task function
    let task = get_task();

    // // Init the public output: a hashmap where we store rewards due to each device
    let mut rewards_map: HashMap<_, u64> = HashMap::new();

    // Process each message in the task
    for data in task.datas.iter() {
         let message: DeviceMessage = serde_json::from_str(data).expect("Failed to deserialize message");

         // Calculate the rewards
         let reward = rewards_map.entry(message.device_id).or_insert(0 as u64);
         *reward = *reward + BASE_REWARD + message.connections * REWARD_PER_CONNECTION;
    }

    // // Serialize the result hashmap and send it back to the host
    let stringified_res = serde_json::to_string(&rewards_map).unwrap();
    env::commit(&stringified_res);
    
    println!("Guest execution completed on {} messages. Proof generation starts now...", task.datas.len());
}


fn get_task() -> W3bstreamTask {
    // Read all necessary task-related inputs from the environment
    let project_id: u64 = env::read();
    env::log(&format!("project_id {}", project_id));
    let task_id: u64 = env::read();
    env::log(&format!("task_id {}", task_id));
    let client_id: String = env::read();
    env::log(&format!("client_id {}", client_id));
    let sequencer_sign: String = env::read();
    env::log(&format!("sequencer_sign {}", sequencer_sign));
    let datas: Vec<String> = env::read();
    env::log(&format!("datas {:?}", datas));

    // Return a Task struct containing all the read values
    W3bstreamTask {
        project_id,
        task_id,
        client_id,
        sequencer_sign,
        datas,
    }
}