# SmartSpectra SDK for iOS

## Integration Guide Overview

This guide provides instructions for integrating and utilizing the Presage SmartSpectra SDK for Swift Package Manager (SPM) publicly hosted at [SmartSpectra SDK](https://github.com/Presage-Security/SmartSpectra/tree/main/swift/sdk) to measure physiology metrics using an Apple mobile device's camera.

The app contained in this repo is an example of using the SmartSpectra SDK and should run out of the box after adding [SmartSpectra Swift SDK](https://github.com/Presage-Security/SmartSpectra/tree/main/swift/sdk) and adding an API key.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start for iOS demo app](#quick-start-for-ios-demo-app)
- [Integration in your app](#integration-in-your-own-app)
- [Usage](#usage)
- [Bugs & Troubleshooting](#bugs--troubleshooting)

## Prerequisites

- iOS 15.0 or later
- Xcode 15.0 or later
- Not usable with emulators or the Xcode simulator

### Authentication

You'll need an API key or Oauth config to use the SmartSpectra Swift SDK. You can register for an account and obtain an API key at <https://physiology.presagetech.com>.

- **API Key**: Add your API key to the [ContentView.swift](samples/demo-app/ContentView.swift) file by replacing the placeholder `"YOUR_API_KEY_HERE"`.
- **Oauth Config**: If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root directory.

## Quick Start for iOS demo app

1. Clone the repository and open the SmartSpectra workspace in Xcode:

    ```bash
    git clone https://github.com/Presage-Security/SmartSpectra/
    open smartspectra/swift/SmartSpectra.xcworkspace
    ```

2. Select the demo app target in Xcode.
3. If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root. If you would like to use API key instead, navigate to [ContentView.swift](samples/demo-app/ContentView.swift) and replace the placeholder API key with your actual API key.
4. Setup the signing and capabilities for the demo app target in Xcode. Make sure to select your development team and set a unique bundle identifier.
5. Connect your iOS device to your computer.
6. Select your device as the target in Xcode.
7. Click the "Run" button in Xcode to build and run the demo app on your device.
8. Follow the on-screen instructions in the app to conduct a measurement and view the results.
9. If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.

## Integration in your own app

### Swift Package Manager

The Swift Package Manager (SPM) is a tool for managing the distribution of Swift code. It automates the process of downloading, compiling, and linking dependencies.

To add SmartSpectra Swift SDK as a dependency to your Xcode project using SPM, follow either of these two sets of steps within Xcode:

- Method 1: Go to File -> "Add Package Dependencies..."
In the "Search or Enter Package URL" field, enter the URL "<https://github.com/Presage-Security/SmartSpectra>"
For the "Dependency Rule," select "Branch" and then "main."
For "Add to Target," select your project.

- Method 2: Open your project in Xcode.  Select your project in the Project Navigator, then click on the project in the Project/Targets Pane. Go to the Package Dependencies Tab, then click the "+" button.

  - **Note**: Some Version of Xcode Navigate to File > Swift Packages > Add Package Dependency
    Paste the repository URL for SmartSpectra SDK in the search bar and press Enter. The URL is <https://github.com/Presage-Security/SmartSpectra>.
    Select Add Package

## Usage

### Example Code

Please refer to [ContentView.swift](samples/demo-app/ContentView.swift) for example usage and plotting of a pulse pleth waveform and breathing waveform.

- **Note**: to use this example repo make sure to under "Signing and Capabilities" of Targets "demo-app" to set:
- Team: Your desired developer profile
- Bundle Identifier: Your desired bundle identifier such as: `com.johnsmith.smartspectratest`
- If you are not a registered developer for the App Store follow the prompt to navigate to Settings > General > VPN & Device Management, then select your developer App certificate to trust it on your iOS device.

### Integrate the SmartSpectra View

You need to integrate the `SmartSpectraView` into your app which is composed of

- A button that allows the user to conduct a measurement and compute physiology metrics
- A resultview that shows the strict breathing rate and pulse rate after the measurement

Here's a minimal example using SwiftUI:

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    init() {
        // (Required), If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // (Required), Deprecated. set apiKey. API key from https://physiology.presagetech.com. Leave default if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)

    }

    var body: some View {
        // add smartspectra view
        SmartSpectraView()
   }
}
```

If you want a lot more customization you can do so in the init function of your view as shown below. See [ContentView.swift](samples/demo-app/ContentView.swift) for implementation example.

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    // Set the initial camera position. Can be set to .front or .back. Defaults to .front
    @State var cameraPosition: AVCaptureDevice.Position = .front
    // Set the initial smartSpectraMode. Can be set to .spot or .continuous. Defaults to .continuous
    @State var smartSpectraMode: SmartSpectraMode = .continuous
    // Set the initial measurement duration. Valid range for measurement duration is between 20.0 and 120.0. Defaults to 30.0
    @State var measurementDuration: Double = 30.0

    // App display configurations
    let isCustomizationEnabled: Bool = true
    let isFaceMeshEnabled: Bool = true

    init() {
        // (Required), If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // (Required), Deprecated. set apiKey. API key from https://physiology.presagetech.com. Leave default if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)

        // (optional) toggle display of camera and smartspectra mode controls in screening view
        sdk.showControlsInScreeningView(isCustomizationEnabled)
        // (Optional), set smartSpectraMode. Can be set to .spot or .continuous. Defaults to .continuous
        sdk.setSmartSpectraMode(smartSpectraMode)
        // (Optional), set measurementDuration. Valid range for measurement duration is between 20.0 and 120.0
        sdk.setMeasurementDuration(measurementDuration)
        // (Optional), set showFPS. To show fps in the screening view
        sdk.setShowFps(false)
        // (Optional), set recordingDelay. To set a initial countdown timer before recording starts. Defaults to 3
        sdk.setRecordingDelay(3)
        // (Optional), set cameraPosition. To set the camera position. Can be set to .front or .back. Defaults to .front
        sdk.setCameraPosition(cameraPosition)
    }

    var body: some View {
        // add smartspectra view
        SmartSpectraView()
   }
}
```

## SmartSpectra Mode

The SmartSpectra SDK supports two modes:

- Spot Mode (`SmartSpectraMode.spot`): In this mode, the SDK will take a single measurement for the specified duration.
- Continuous Mode (`SmartSpectraMode.continuous`): In this mode, the SDK will continuously take measurements for the specified duration. Currently defaults to infinite duration and manual stop. During continuous mode, live pulse and breathing rate is displayed; along with live plots of the pulse and breathing trace.

## Switching SmartSpectra Mode, Measurement Duration, and Camera Position at Runtime

See examples of how you can implement UI elements or programmatically change the SmartSpectra mode, measurement duration, and camera position at runtime in [ContentView.swift](samples/demo-app/ContentView.swift).

> [!IMPORTANT]
> You need to enter your API key string at `"YOUR_API_KEY_HERE"`. Optionally, you can also configure measurement type, duration, whether to show frame per second (fps) during screening.

## Using the SmartSpectra SDK headless

SmartSpectrSDK can be used headless without the SmartSpectraView. This is useful if you want to use the SDK to collect data in the background without displaying the camera view. It is also useful, if you want to display any other content while monitoring the vitals in the background. To get started with the headless mode, you will need to use `SmartSpectraVitalsProcessor` to control the vitals processing, while using the `SmartSpectraSwiftSDK` to configure and monitor the vitals.

Here's a minimal example to get started with the headless mode (see [HeadlessSDKExample.swift](samples/demo-app/HeadlessSDKExample.swift)):

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct HeadlessSDKExample: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var isVitalMonitoringEnabled: Bool = false

    init() {
        // (Required), If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // (Required), Deprecated. set apiKey. API key from https://physiology.presagetech.com. Leave default if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)
    }

    var body: some View {
        VStack {
            GroupBox(label: Text("Vitals")) {
                ContinuousVitalsPlotView()
                Grid {
                    GridRow {
                        Text("Status: \(vitalsProcessor.statusHint)")
                    }
                    GridRow {
                        HStack {
                            Text("Vitals Monitoring")
                            Spacer()
                            Button(isVitalMonitoringEnabled ? "Stop": "Start") {
                                isVitalMonitoringEnabled.toggle()
                                if(isVitalMonitoringEnabled) {
                                    startVitalsMonitoring()
                                } else {
                                    stopVitalsMonitoring()
                                }
                            }
                        }
                    }
                }
                .padding(.horizontal, 10)
            }
        }
    }

    func startVitalsMonitoring() {
        vitalsProcessor.startProcessing()
        vitalsProcessor.startRecording()
    }

    func stopVitalsMonitoring() {
        vitalsProcessor.stopProcessing()
        vitalsProcessor.stopRecording()

    }
}
```

### Extracting and Using Metrics Data

The `MetricsBuffer` is the main struct generated using [swift-protobuf](https://github.com/apple/swift-protobuf) that contains all metrics data. You can access it through a `@ObservedObject` instance of `SmartSpectraSwiftSDK.shared`. This way any update to the metrics data will automatically trigger a UI update.

**Usage Example:**

```swift
@ObservedObject var shared = SmartSpectraSwiftSDK.shared

if let metrics = sdk.metricsBuffer {
  // Use the metrics

  // Access pulse data
  metrics.pulse.rate.forEach { measurement in
      print("Pulse rate value: \(measurement.value), time: \(measurement.time), confidence: \(measurement.confidence)")
  }

  // Access breathing data
  metrics.breathing.rate.forEach { rate in
      print("Breathing rate: \(rate.value), time: \(rate.time), confidence: \(rate.confidence)")
  }
}


```

### Detailed `MetricsBuffer` Struct Descriptions

> [!TIP]
> If you need to use the types directly, the MetricsBuffer and corresponding structs are under the `Presage_Physiology` namespace. You can type alias it from the `Presage_Physiology_MetricsBuffer` to `MetricsBuffer` for easier usage:
>
> ```swift
> typealias MetricsBuffer = Presage_Physiology_MetricsBuffer
> ```

Metrics Buffer contains the following parent structs:

```swift
struct MetricsBuffer {
    var pulse: Pulse
    var breathing: Breathing
    var bloodPressure: BloodPressure
    var face: Face
    var metadata: Metadata
}
```

### Measurement Types

- **`Measurement` Struct** : Represents a measurement with time and value:

```swift
struct Measurement {
    var time: Float
    var value: Float
    var stable: Bool
}
```

- **`MeasurementWithConfidence` Struct** : Includes confidence with the measurement:

```swift
struct MeasurementWithConfidence {
    var time: Float
    var value: Float
    var stable: Bool
    var confidence: Float
}
```

- **`DetectionStatus` Struct** :Used for events like apnea or face detection (blinking/talking):

```swift
struct DetectionStatus {
    var time: Float
    var detected: Bool
    var stable: Bool
}
```

#### Metric Types

- **`Pulse` Struct** : Contains pulse-related measurements, including rate, trace, and strict values:

```swift
struct Pulse {
    var rate: [MeasurementWithConfidence]
    var trace: [Measurement]
    var strict: Strict
}
```

- **`Breathing` Struct** : Handles breathing-related data with upper and lower traces, amplitude, apnea status, and other metrics:

```swift
struct Breathing {
    var rate: [MeasurementWithConfidence]
    var upperTrace: [Measurement]
    var lowerTrace: [Measurement]
    var amplitude: [Measurement]
    var apnea: [DetectionStatus]
    var respiratoryLineLength: [Measurement]
    var inhaleExhaleRatio: [Measurement]
    var strict: Strict
}
```

- **`BloodPressure` Struct** : Handles blood pressure measurements:

> [!CAUTION]
> Currently not available publicly, currently returned results are a duplicate of pulse pleth

```swift
struct BloodPressure {
    var phasic: [MeasurementWithConfidence]
}
```

- **`Face` Struct** : Includes detection statuses for blinking and talking:

```swift
struct Face {
    var blinking: [DetectionStatus]
    var talking: [DetectionStatus]
}
```

- **`Metadata` Struct** : Includes metadata information:

```swift
struct Metadata {
    var id: String
    var uploadTimestamp: String
    var apiVersion: String
}
```

#### Encoding and Decoding Protobuf Messages

To serialize `MetricsBuffer` into binary format:

```swift
do {
    let data = try metrics.serializedData()
    // Send `data` to your backend or save it
} catch {
    print("Failed to serialize metrics: \(error)")
}
```

To decode binary protobufdata into `MetricsBuffer`:

```swift
do {
    let decodedMetrics = try MetricsBuffer(serializedBytes: data)
    // Use `decodedMetrics` as needed
} catch {
    print("Failed to decode metrics: \(error)")
}
```

### Displaying face mesh points

You can display the face mesh points by following the example in [ContentView.swift](samples/demo-app/ContentView.swift)

```Swift
if !sdk.meshPoints.isEmpty {
   // Visual representation of mesh points
   GeometryReader { geometry in
         ZStack {
            ForEach(Array(sdk.meshPoints.enumerated()), id: \.offset) { index, point in
               Circle()
                     .fill(Color.blue)
                     .frame(width: 3, height: 3)
                     .position(x: CGFloat(point.x) * geometry.size.width / 1280.0,
                           y: CGFloat(point.y) * geometry.size.height / 1280.0)
            }
         }
   }
   .frame(width: 400, height: 400) // Adjust the height as needed
}
```

Since the mesh points are published you can also use `combine` to subscribe to the mesh points to add a custom callback to further process the mesh points.

## Device Orientation

We do not recommend landscape support. We recommend removing the "Landscape Left," "Landscape Right," and "Portrait Upside Down" modes from your supported interface orientations.

## Bugs & Troubleshooting

For additional support, contact <[support@presagetech.com](mailto:support@presagetech.com)> or submit a [Github Issue](https://github.com/Presage-Security/SmartSpectra/issues)
