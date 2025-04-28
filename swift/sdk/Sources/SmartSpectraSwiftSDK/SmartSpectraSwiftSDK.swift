// The Swift Programming Language
// https://docs.swift.org/swift-book
import Foundation
import Combine

import PresagePreprocessing
import AVFoundation
import SwiftUICore

public typealias MetricsBuffer = Presage_Physiology_MetricsBuffer

// Expose the data provider through your SDK's API
public class SmartSpectraSwiftSDK: ObservableObject {
    public static let shared = SmartSpectraSwiftSDK()
    @Published public var meshPoints: [(x: Int16, y: Int16)] = []
    @Published public var metricsBuffer: MetricsBuffer? {
        didSet {
            if config.smartSpectraMode == .spot {
                //TODO: 10/24/24: Update this for all result once strict results return for continuous
                updateResultText()
            }
        }
    }

    @Published internal var resultText: String = "No Results\n..."
    @Published internal var resultErrorText: String = ""

    internal var config: SmartSpectraSdkConfig = SmartSpectraSdkConfig.shared

    private init(apiKey: String = "", showFps: Bool = false) {
        self.config.apiKey = apiKey
        self.config.showFps = showFps

        // initiate app auth flow
        AuthHandler.shared.startAuthWorkflow()

        // Uncomment this to use test server. Use with extreme caution. do not use this in production
        // useTestServer()
    }


    public func setShowFps(_ showFps: Bool) {
        self.config.showFps = showFps
    }

    public func setSmartSpectraMode(_ mode: SmartSpectraMode) {
        guard self.config.smartSpectraMode != mode else { return }
        self.config.smartSpectraMode = mode
    }

    public func setMeasurementDuration(_ duration: Double) {
        self.config.measurementDuration = duration
    }

    public func setRecordingDelay(_ delay: Int) {
        self.config.recordingDelay = delay
    }

    public func setCameraPosition(_ cameraPosition: AVCaptureDevice.Position) {
        guard self.config.cameraPosition != cameraPosition else { return }
        self.config.cameraPosition = cameraPosition
    }

    public func showControlsInScreeningView(_ showControls: Bool) {
        self.config.showControlsInScreeningView = showControls
    }

    public func setApiKey(_ apiKey: String) {
        self.config.apiKey = apiKey
    }

    private func updateResultText() {
        guard let metricsBuffer = metricsBuffer, metricsBuffer.isInitialized else {
            resultText = "No Results\n..."
            return
        }

        let strictPulseRate = round(metricsBuffer.pulse.strict.value)
        let strictBreathingRate = round(metricsBuffer.breathing.strict.value)
        let strictPulseRateInt = Int(strictPulseRate)
        let strictBreathingRateInt = Int(strictBreathingRate)

        let pulseRateText = "Pulse Rate: \(strictPulseRateInt == 0 ? "N/A": "\(strictPulseRateInt) BPM")"
        let breathingRateText = "Breathing Rate: \(strictBreathingRateInt == 0 ? "N/A": "\(strictBreathingRateInt) BPM")"
        resultText = "\(breathingRateText)\n\(pulseRateText)"

        if strictPulseRateInt == 0 || strictBreathingRateInt == 0 {
            // TODO: 9/30/24 Replace print with Swift Logging
            print("Insufficient data for measurement. Strict Pulse Rate: \(strictPulseRate), Strict Breathing Rate: \(strictBreathingRate)")
            resultErrorText = "Your data was insufficient for an accurate measurement. Please move to a better-lit location, hold still, and try again. For more guidance, see the tutorial in the dropdown menu of the 'i' icon next to 'Checkup.'"
        } else {
            resultErrorText = ""
        }
    }

    @available(*, deprecated, message: "This method is experimental and should not be used in production. Only use for testing purposes.")
    private func useTestServer() {
        PresagePreprocessing.useTestServer()
    }

    @available(*, deprecated, message: "This method is experimental and should not be used in production. Only use for testing purposes.")
    public func useBetaServer() {
        PresagePreprocessing.setServer(PresageServer.beta)
    }

    internal func updateErrorText(_ errorMessage: String) {

        DispatchQueue.main.async {
            if errorMessage.isEmpty {
                self.resultErrorText = ""
            } else {
                self.resultErrorText = "Error: \(errorMessage)"
            }

        }
    }

}
