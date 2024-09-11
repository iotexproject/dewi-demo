use dewi_core::*;
use rand::Rng;
use serde_json;
use dewi_demo::process_task;
use dewi_demo_methods::DEWI_ID;

fn main() {
    // Prepare task-related data
    let project_id: u64 = 12345;
    let task_id: u64 = 1;
    let client_id = "client_001".to_string();
    let sequencer_sign = "sequencer_sign_123".to_string();

    // Fill in the task with some data
    let mut datas: Vec<String> = Default::default();
    for _ in 0..TASK_SIZE {
        // Create a new message struct
        let message = DeviceMessage {
            client_id: rand::thread_rng().gen_range(0..3).to_string(),
            connections: rand::thread_rng().gen_range(0..4) as u64,
        };

        // Serialize the message to a JSON string
        let serstr: String = serde_json::to_string(&message).unwrap();
        println!("Message: {}", serstr);
        datas.push(serstr);
    }

    // Combine all task-related data into a single string
    let task_data = serde_json::to_string(&datas).unwrap();
    // Prepare the task 
    let test_task = W3bstreamTask {
        project_id,
        task_id,
        client_id,
        sequencer_sign,
        datas,
    };
    println!("Processing task: {}", test_task);

    // Process the task and get the receipt
    let (receipt, _) = process_task(test_task);

    // Verify the receipt, panic if it's wrong
    receipt.verify(DEWI_ID).expect(
        "Code you have proven should successfully verify; did you specify the correct image ID?",
    );
}
