#![doc = include_str!("../README.md")]

use dewi_core::W3bstreamTask;
use chrono::Local;
use dewi_demo_methods::DEWI_ELF;
use risc0_zkvm::{default_prover, ExecutorEnv, Receipt};

// This function processes the task by sending it to the zkVM and obtaining a proof (receipt).
pub fn process_task(task: W3bstreamTask) -> (Receipt, String) {
    // Create an executor environment and write the task string to it
    let env = ExecutorEnv::builder()
        .write(&task)
        .unwrap()
        .build()
        .unwrap();

    // Obtain the default prover.
    let prover = default_prover();

    // Produce a receipt by proving the specified ELF binary.
    let start = std::time::Instant::now();
    let date = Local::now();
    println!("Start proving at {}", date.format("%Y-%m-%d %H:%M:%S"));

    // Prove the execution and get the receipt
    let receipt = prover.prove(env, DEWI_ELF).unwrap();
    println!("Proving time: {:?}", start.elapsed());

    // Extract the journal from the receipt
    let output: String = receipt.journal.decode().unwrap();

    // Print the public output from the journal
    println!("Proof of guest execution generated! Public output from journal:");
    println!("Journal: {}", output);

    (receipt, output)
}
