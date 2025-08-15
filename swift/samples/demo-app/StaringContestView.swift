//
//  StaringContestView.swift
//  demo-app
//
//  Created by Claude on 8/14/25.
//

import SwiftUI
import SmartSpectraSwiftSDK

@available(iOS 16.0, *)
struct StaringContestView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared

    @State private var gameState: GameState = .waiting
    @State private var startTime: Date = Date()
    @State private var elapsedTime: TimeInterval = 0
    @State private var timer: Timer?
    @State private var hasDetectedBlink: Bool = false

    enum GameState {
        case waiting
        case running
        case gameOver
    }

    var body: some View {
        ZStack {
            // Background color
            (gameState == .gameOver ? Color.red : Color.black)
                .ignoresSafeArea()

            VStack(spacing: 40) {
                switch gameState {
                case .waiting:
                    waitingView
                case .running:
                    runningView
                case .gameOver:
                    gameOverView
                }
            }
        }
        .preferredColorScheme(.dark)
        .onAppear {
            setupSDK()
        }
        .onDisappear {
            stopGame()
        }
    }

    private var waitingView: some View {
        VStack(spacing: 40) {
            Text("STARING CONTEST")
                .font(.largeTitle)
                .fontWeight(.bold)
                .foregroundColor(.white)

            Text("Don't blink!")
                .font(.title2)
                .foregroundColor(.white)

            Button(action: startGame) {
                Text("START")
                    .font(.title)
                    .fontWeight(.bold)
                    .foregroundColor(.black)
                    .frame(width: 200, height: 60)
                    .background(Color.white)
                    .cornerRadius(30)
            }
        }
    }

    private var runningView: some View {
        VStack(spacing: 40) {
            Text("DON'T BLINK!")
                .font(.largeTitle)
                .fontWeight(.bold)
                .foregroundColor(.white)

            Text(formatTime(elapsedTime))
                .font(.system(size: 60, design: .monospaced))
                .fontWeight(.bold)
                .foregroundColor(.white)

            Button(action: stopGame) {
                Text("STOP")
                    .font(.title2)
                    .fontWeight(.semibold)
                    .foregroundColor(.black)
                    .frame(width: 120, height: 40)
                    .background(Color.white)
                    .cornerRadius(20)
            }
        }
    }

    private var gameOverView: some View {
        VStack(spacing: 40) {
            Text("YOU BLINKED!")
                .font(.largeTitle)
                .fontWeight(.bold)
                .foregroundColor(.white)

            Text("Final Time:")
                .font(.title2)
                .foregroundColor(.white)

            Text(formatTime(elapsedTime))
                .font(.system(size: 48, design: .monospaced))
                .fontWeight(.bold)
                .foregroundColor(.white)

            Button(action: resetGame) {
                Text("RESTART")
                    .font(.title2)
                    .fontWeight(.bold)
                    .foregroundColor(.red)
                    .frame(width: 200, height: 50)
                    .background(Color.white)
                    .cornerRadius(25)
            }
        }
    }

    private func setupSDK() {
        // SDK is already configured in ContentView, so we don't need to set API key again
    }

    private func startGame() {
        // Reset the blink detection flag
        hasDetectedBlink = false

        gameState = .running
        startTime = Date()
        elapsedTime = 0

        // Restart the vitals processor to get fresh data
        vitalsProcessor.stopProcessing()
        vitalsProcessor.stopRecording()

        // Small delay to ensure clean restart
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
            self.vitalsProcessor.startProcessing()
            self.vitalsProcessor.startRecording()
        }

        // Start the timer for updating elapsed time
        timer = Timer.scheduledTimer(withTimeInterval: 0.01, repeats: true) { _ in
            updateElapsedTime()
            checkForBlink()
        }
    }

    private func stopGame() {
        timer?.invalidate()
        timer = nil
        vitalsProcessor.stopProcessing()
        vitalsProcessor.stopRecording()

        if gameState == .running {
            gameState = .waiting
        }
    }

    private func resetGame() {
        gameState = .waiting
        elapsedTime = 0
        hasDetectedBlink = false
    }

    private func updateElapsedTime() {
        elapsedTime = Date().timeIntervalSince(startTime)
    }

    private func checkForBlink() {
        guard gameState == .running && !hasDetectedBlink else { return }

        // Check if we have metrics data with blink detection
        if let metrics = sdk.metricsBuffer {
            let face = metrics.face

            print("Checking for blinks. Total blinks in buffer: \(face.blinking.count)")

            // Look for any detected, stable blinks
            for blinkDetection in face.blinking {
                if blinkDetection.detected && blinkDetection.stable {
                    print("Blink detected! detected: \(blinkDetection.detected), stable: \(blinkDetection.stable), time: \(blinkDetection.time)")

                    // Set flag to prevent multiple detections
                    hasDetectedBlink = true

                    // Trigger game over
                    gameState = .gameOver
                    timer?.invalidate()
                    timer = nil
                    vitalsProcessor.stopProcessing()
                    vitalsProcessor.stopRecording()
                    return
                }
            }
        }
    }

    private func formatTime(_ time: TimeInterval) -> String {
        let milliseconds = Int((time.truncatingRemainder(dividingBy: 1)) * 1000)
        let seconds = Int(time) % 60
        let minutes = Int(time) / 60

        if minutes > 0 {
            return String(format: "%d:%02d.%03d", minutes, seconds, milliseconds)
        } else {
            return String(format: "%d.%03d", seconds, milliseconds)
        }
    }
}

#Preview {
    if #available(iOS 16.0, *) {
        StaringContestView()
    }
}

