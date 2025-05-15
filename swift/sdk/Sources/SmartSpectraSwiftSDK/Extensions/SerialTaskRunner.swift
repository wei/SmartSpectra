//
//  SerialTaskRunner.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 4/29/25.
//


internal actor SerialTaskRunner {
    private var isRunning = false
    private var queue: [() async -> Void] = []

    func enqueue(_ job: @escaping () async -> Void) {
        queue.append(job)
        if !isRunning { Task { await runNext() } }
    }

    private func runNext() async {
        guard !queue.isEmpty else { isRunning = false; return }
        isRunning = true
        let job = queue.removeFirst()
        await job()
        await runNext()
    }
}
