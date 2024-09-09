// Define the message type

use serde::{Serialize, Deserialize};
pub const TASK_SIZE: usize = 10;
pub const BASE_REWARD: u64 = 1;
pub const REWARD_PER_CONNECTION: u64 = 1;

#[derive(Serialize, Deserialize)]
pub struct DeviceMessage {
    pub client_id: u64,
    pub connections: u64
}

// Define a struct to hold the W3bstream task information
#[derive(Serialize, Deserialize)]
pub struct W3bstreamTask {
    pub project_id: u64,
    pub task_id: u64,
    pub client_id: String,
    pub sequencer_sign: String,
    pub datas: Vec<String>,
}

// Implement the Display trait for the W3bstreamTask struct
impl std::fmt::Display for W3bstreamTask {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "W3bstreamTask {{ project_id: {}, task_id: {}, client_id: {}, sequencer_sign: {}, datas: {:?} }}", self.project_id, self.task_id, self.client_id, self.sequencer_sign, self.datas)
    }
}

