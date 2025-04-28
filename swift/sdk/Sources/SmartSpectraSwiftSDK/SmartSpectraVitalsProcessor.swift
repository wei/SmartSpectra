//
//  SmartSpectraProcessor.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/3/25.
//

import Foundation
import PresagePreprocessing
import CoreImage
import UIKit
import SwiftUICore

enum PresageProcessingStatus {
    case idle
    case processing
    case processed
    case error
}

class ImageConverter {
    private let context = CIContext(options: nil)
    private let queue = DispatchQueue(label: "image.convert.queue")

    func convertAsync(pixelBuffer: CVPixelBuffer, completion: @escaping (UIImage?) -> Void) {
        queue.async {
            autoreleasepool {
                let ciImage = CIImage(cvPixelBuffer: pixelBuffer)
                let rect = CGRect(x: 0, y: 0,
                                  width: CVPixelBufferGetWidth(pixelBuffer),
                                  height: CVPixelBufferGetHeight(pixelBuffer))
                guard let cgImage = self.context.createCGImage(ciImage, from: rect) else {
                    completion(nil)
                    return
                }
                completion(UIImage(cgImage: cgImage))
            }
        }
    }
}

public class SmartSpectraVitalsProcessor: NSObject, ObservableObject {
    public static let shared = SmartSpectraVitalsProcessor()
    @Published var imageOutput: UIImage?
    @Published var processingStatus: PresageProcessingStatus = .idle
    @Published var counter: Double = 0
    @Published var fps: Int = 0
    @Published public var lastStatusCode: StatusCode = .processingNotStarted
    @Published public var statusHint: String = ""
    @Published var isRecording: Bool = false
    var presageProcessing: PresagePreprocessing = PresagePreprocessing()
    var lastTimestamp: Int?
    var fpsValues: [Int] = []
    let movingAveragePeriod = 10
    private var sdk: SmartSpectraSwiftSDK
    private var avCaptureDeviceManager: AVCaptureDeviceManager = AVCaptureDeviceManager.shared
    @State private var coreIsRunning: Bool = false // TODO: perhaps add a processing status for not ready
    private let imageConverter = ImageConverter()
    private weak var authHandler: AuthHandler?

    private override init(){
        sdk = SmartSpectraSwiftSDK.shared
        authHandler = AuthHandler.shared
        super.init()
        presageProcessing.delegate = self
    }

    internal func setRecordingState(_ state: Bool) {
        presageProcessing.buttonStateChanged(inFramework: state)
        DispatchQueue.main.async {
            self.isRecording = state
        }
    }

    internal func changeProcessingMode(_ mode: SmartSpectraMode) {
        guard presageProcessing.mode != mode.presageMode else { return }
        presageProcessing.mode = mode.presageMode
        sdk.config.smartSpectraMode = mode
    }

    internal func setProcessingCameraPosition(_ position: AVCaptureDevice.Position) {
        guard presageProcessing.cameraPosition != position else { return }
        presageProcessing.cameraPosition = position
        sdk.config.cameraPosition = position
    }

    internal func setApiKey(_ apiKey: String) {
        presageProcessing.apiKey = apiKey
        sdk.config.apiKey = apiKey
    }

    internal func setSpotDuration(_ duration: Double) {
        presageProcessing.spotDuration = duration
        sdk.config.measurementDuration = duration
    }

    private func setupProcessing() {
        let oauth_enabled  = authHandler?.isOauthEnabled ?? false

        if !oauth_enabled {
            guard let apiKey = sdk.config.apiKey, !apiKey.isEmpty else { fatalError("API key missing") }
            setApiKey(apiKey)
        }

        changeProcessingMode(sdk.config.smartSpectraMode)
        setProcessingCameraPosition(sdk.config.cameraPosition)
        if(sdk.config.smartSpectraMode == .spot) {
            setSpotDuration(sdk.config.measurementDuration)
        }
    }

    public func startProcessing() {
        if coreIsRunning {
            stopProcessing()
        }
        processingStatus = .idle
        setupProcessing()
        presageProcessing.start()
        coreIsRunning = true
    }

    public func stopProcessing() {
        processingStatus = .idle
        presageProcessing.stop()
        imageOutput = nil
        coreIsRunning = false
    }

    // Method to start recording
    public func startRecording() {
        UIApplication.shared.isIdleTimerDisabled = true
        avCaptureDeviceManager.lockCameraSettings()
        setRecordingState(true)
    }

    // Method to stop recording
    public func stopRecording() {
        UIApplication.shared.isIdleTimerDisabled = false
        avCaptureDeviceManager.unlockCameraSettings()
        setRecordingState(false)
    }

}

extension SmartSpectraVitalsProcessor: PresagePreprocessingDelegate {

    public func frameWillUpdate(_ tracker: PresagePreprocessing!, didOutputPixelBuffer pixelBuffer: CVPixelBuffer!, timestamp: Int) {
        //TODO: optimize to not update image when in headless mode
        // Convert the pixel buffer to UIImage asynchronously and publish it.

        //TODO: Need better approach here: Conversion to UIImage is not very efficient, and causes a lot of memory pressure.
        imageConverter.convertAsync(pixelBuffer: pixelBuffer) { image in
            if let image = image {
                DispatchQueue.main.async {
                    self.imageOutput = image
                }
            }
        }

    }

    public func frameDidUpdate(_ tracker: PresagePreprocessing!, didOutputPixelBuffer pixelBuffer: CVPixelBuffer!) {
        // TODO: figure out if we need to keep this or remove it
    }

    public func statusCodeChanged(_ tracker: PresagePreprocessing!, statusCode: StatusCode) {

        if statusCode != lastStatusCode {
            DispatchQueue.main.async {
                self.lastStatusCode = statusCode
                self.statusHint = tracker.getStatusHint(statusCode)
            }
        }
        if sdk.config.smartSpectraMode == .spot && sdk.config.showFps {
            // update fps based on status code in spot mode
            updateFps()
        }

    }

    public func metricsBufferChanged(_ tracker: PresagePreprocessing!, serializedBytes: Data) {
        do {
            // Deserialize the data directly into the Swift Protobuf object
            let metricsBuffer = try MetricsBuffer(serializedBytes: serializedBytes)
            // print("Received metrics buffer. metadata: \(String(describing: metricsBuffer.metadata))")
            //            print("Pulse: \(String(describing: metricsBuffer.pulse.rate.last?.value)), Breathing: \(String(describing: metricsBuffer.breathing.rate.last?.value))")
            // update metrics buffer
            if sdk.config.smartSpectraMode == .spot {
                DispatchQueue.main.async {
                    self.processingStatus = .processed
                }
            }

            DispatchQueue.main.async {
                self.sdk.metricsBuffer = metricsBuffer
            }

            if sdk.config.smartSpectraMode == .continuous && sdk.config.showFps {
                //update fps based on metricsBuffer in continuous mode
                updateFps()
            }

        } catch {
            print("Failed to deserialize MetricsBuffer: \(error.localizedDescription)")
        }
    }

    public func timerChanged(_ timerValue: Double) {
        if counter != timerValue {
            DispatchQueue.main.async {
                self.counter = timerValue
                if self.counter == 0.0 && self.processingStatus == .idle {
                    self.processingStatus = .processing
                }
            }
        }
    }

    public func receiveDenseFacemeshPoints(_ points: [NSNumber]) {
        // Convert and unflatten the array into tuples of (Int16, Int16)
        let unflattenedPoints = stride(from: 0, to: points.count, by: 2).map { (points[$0].int16Value, points[$0 + 1].int16Value) }

        // Asynchronously update shared data manager
        DispatchQueue.main.async {
            self.sdk.meshPoints = unflattenedPoints
        }
    }

    public func handleGraphError(_ error: Error) {
        print("Error in vital processing: \(error)")
        self.sdk.updateErrorText("Internal error occurred. Check your internet connection and retry. If it happens repeatedly contact customer support.")
        //clear metrics buffer
        DispatchQueue.main.async {
            self.processingStatus = .error
            self.sdk.metricsBuffer = nil
        }
    }

    fileprivate func updateFps() {
        // update fps
        let currentTime = Int(Date().timeIntervalSince1970 * 1000)

        if let lastTimestamp = lastTimestamp {
            let deltaTime = currentTime - lastTimestamp

            fpsValues.append(deltaTime)
            if fpsValues.count > movingAveragePeriod {
                fpsValues.removeFirst()
            }
            // TODO: 10/28/24: Fix this further upstream so this isn't necessary
            let averageDeltaTime = max(Double(fpsValues.reduce(0, +)) / Double(fpsValues.count), 0.0001)

            DispatchQueue.main.async {
                self.fps = Int(round(1000 / averageDeltaTime))
            }
        }
        lastTimestamp = currentTime
    }
}
