import Foundation
import AVFoundation
import PresagePreprocessing

public enum SmartSpectraMode {
    case spot
    case continuous

    // Internal helper to map to PresageMode
    internal var presageMode: PresageMode {
        switch self {
        case .spot:
            return .spot
        case .continuous:
            return .continuous
        }
    }
}

internal class SmartSpectraSdkConfig: ObservableObject {
    internal static let shared = SmartSpectraSdkConfig()
    @Published internal var smartSpectraMode: SmartSpectraMode
    @Published internal var measurementDuration: Double
    internal var apiKey: String?
    internal var showFps: Bool = false
    internal var recordingDelay: Int = 3
    internal var cameraPosition: AVCaptureDevice.Position = .front
    internal var showControlsInScreeningView: Bool = true
    internal var headlessMode: Bool = false

    // defaults to 30 second spot if configuration is not supplied
    internal init(smartSpectraMode: SmartSpectraMode = .continuous, duration: Double =  30.0) {
        self.smartSpectraMode = smartSpectraMode
        self.measurementDuration = clipValue(duration, minValue: 20.0, maxValue: 120.0)
    }
}

fileprivate func clipValue(_ value: Double, minValue: Double, maxValue: Double) -> Double {
    if value < minValue {
        Logger.log("Warning: duration \(value) is below the minimum value. Clipping to \(minValue).")
        return minValue
    } else if value > maxValue {
        Logger.log("Warning: duration \(value) is above the maximum value. Clipping to \(maxValue).")
        return maxValue
    } else {
        return value
    }
}
