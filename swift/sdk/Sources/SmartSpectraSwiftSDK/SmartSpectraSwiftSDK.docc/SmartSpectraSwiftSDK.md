# ``SmartSpectraSwiftSDK``

A Swift SDK for measuring physiology metrics using mobile device's camera.

## Overview

The SmartSpectra SDK provides a comprehensive solution for capturing and analyzing physiological data including pulse rate, breathing patterns, and other vital signs using computer vision and signal processing techniques.

The SDK offers both a complete UI solution with ``SmartSpectraView`` and a headless mode for custom implementations, making it flexible for various app architectures and use cases.

## Key Features

- **Cardiac Waveform**
  Real-time pulse pleth waveform supporting calculation of pulse rate and heart rate variability.

- **Breathing Waveform**
  Real-time breathing waveform supporting biofeedback, breathing rate, inhale/exhale ratio, breathing amplitude, apnea detection, and respiratory line length.

- **Myofacial Analysis**
  Supporting face-point analysis, iris tracking, blinking detection, and talking detection.

- **Integrated Quality Control**
  Confidence and stability metrics providing insight into the confidence in the signal. User feedback on imaging conditions to support successful use.

- **Camera Selection**
  Front or rear facing camera selection.

- **Continuous or Spot Measurement**
  Options for continuous measurements or variable time window spot measurements to support varied use cases.

## Prerequisites

- iOS 15.0 or later
- Xcode 15.0 or later
- Physical iOS device (not compatible with emulators or Xcode simulator)
- Valid API key or OAuth configuration from [PresageTech Developer Portal](https://physiology.presagetech.com/)

## Quick Start

### 1. Add the Package Dependency

Add SmartSpectra SDK to your Xcode project using Swift Package Manager:

1. In Xcode, go to **File** → **Add Package Dependencies...**
2. Enter the repository URL: `https://github.com/Presage-Security/SmartSpectra`
3. Select **Branch: main** for the dependency rule
4. Add to your target

### 2. Basic Integration

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    init() {
        // Set your API key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)
    }

    var body: some View {
        SmartSpectraView()
    }
}
```

### 3. Advanced Configuration

```swift
init() {
    let apiKey = "YOUR_API_KEY_HERE"
    sdk.setApiKey(apiKey)

    // Configure measurement settings
    sdk.setSmartSpectraMode(.continuous)  // or .spot
    sdk.setMeasurementDuration(30.0)      // 20.0 - 120.0 seconds
    sdk.setCameraPosition(.front)         // or .back
    sdk.setRecordingDelay(3)              // countdown before recording
    sdk.showControlsInScreeningView(true) // show/hide UI controls
}
```

## SDK Modes

### Spot Mode

- Single measurement for specified duration
- Provides final results after measurement completion
- Best for quick health checks

### Continuous Mode

- Real-time monitoring with live updates
- Displays live pulse and breathing traces
- Ideal for extended monitoring sessions

## Headless Usage

For custom UI implementations, use the headless mode:

```swift
@ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared

func startMonitoring() {
    vitalsProcessor.startProcessing()
    vitalsProcessor.startRecording()
}

func stopMonitoring() {
    vitalsProcessor.stopProcessing()
    vitalsProcessor.stopRecording()
}
```

## Data Access

Access measurement data through the metrics buffer:

```swift
@ObservedObject var sdk = SmartSpectraSwiftSDK.shared

if let metrics = sdk.metricsBuffer {
    // Access pulse data
    metrics.pulse.rate.forEach { measurement in
        print("Pulse: \(measurement.value) BPM at \(measurement.time)s")
    }

    // Access breathing data
    metrics.breathing.rate.forEach { rate in
        print("Breathing: \(rate.value) RPM at \(rate.time)s")
    }
}
```

## Authentication

The SDK supports two authentication methods:

### API Key Authentication

Set your API key during initialization:

```swift
sdk.setApiKey("YOUR_API_KEY_HERE")
```

### OAuth Authentication

For TestFlight/App Store releases, place the OAuth configuration plist file in your app's root directory. No additional code required.

## Face Mesh and Detection

Display face mesh points for debugging or custom UI:

```swift
if !sdk.meshPoints.isEmpty {
    GeometryReader { geometry in
        ZStack {
            ForEach(Array(sdk.meshPoints.enumerated()), id: \.offset) { index, point in
                Circle()
                    .fill(Color.blue)
                    .frame(width: 3, height: 3)
                    .position(
                        x: CGFloat(point.x) * geometry.size.width / 1280.0,
                        y: CGFloat(point.y) * geometry.size.height / 1280.0
                    )
            }
        }
    }
}
```

## Data Serialization

Export and import measurement data:

```swift
// Serialize to binary format
do {
    let data = try metrics.serializedData()
    // Save or transmit data
} catch {
    print("Serialization failed: \(error)")
}

// Deserialize from binary data
do {
    let decodedMetrics = try MetricsBuffer(serializedBytes: data)
    // Use decoded metrics
} catch {
    print("Deserialization failed: \(error)")
}
```

## Best Practices

- **Device Orientation**: Recommend portrait mode only for optimal performance
- **Lighting Conditions**: Ensure adequate lighting for accurate measurements
- **Camera Position**: Front camera typically provides better face detection
- **Measurement Duration**: 30-60 seconds recommended for reliable results
- **Background Processing**: Use headless mode for background vital monitoring

## Troubleshooting

- Ensure physical device testing (simulators not supported)
- Verify API key validity and network connectivity
- Check camera permissions in device settings
- Ensure adequate lighting and face visibility
- Review measurement duration and mode settings

## Topics

### Classes

- ``SmartSpectraSwiftSDK``
- ``SmartSpectraVitalsProcessor``

### Views and UI Components

- ``SmartSpectraView``
- ``ContinuousVitalsPlotView``

### Protocols

- ``TimeStamped``

### Type Aliases

- ``Metrics``
- ``MetricsBuffer``

### Enumerations

- ``SmartSpectraMode``

### Main Data Containers

The primary data structures that contain all metrics and metadata.

- ``Presage_Physiology_MetricsBuffer``
- ``Presage_Physiology_Metrics``
- ``Presage_Physiology_Metadata``

### Physiological Metrics

Structures containing vital sign measurements and physiological data.

- ``Presage_Physiology_Pulse``
- ``Presage_Physiology_Breathing``
- ``Presage_Physiology_BloodPressure``

### Face Detection Data

Structures for face-related detection and landmark data.

- ``Presage_Physiology_Face``
- ``Presage_Physiology_Landmarks``

### Measurement Types

Different types of measurement structures with varying data precision.

- ``Presage_Physiology_Measurement``
- ``Presage_Physiology_MeasurementWithConfidence``
- ``Presage_Physiology_DetectionStatus``

### Geometric Data Types

Point structures for 2D and 3D coordinate data.

- ``Presage_Physiology_Point2dFloat``
- ``Presage_Physiology_Point2dInt32``
- ``Presage_Physiology_Point3dFloat``

### Supporting Data Structures

Helper structures that support the main physiological data types.

- ``Presage_Physiology_Strict``
- ``Presage_Physiology_Trace``
