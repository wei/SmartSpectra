# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

**Swift Package Manager (SPM) SDK:**
- `swift build` - Build the SmartSpectraSwiftSDK package (in `sdk/` directory)
- `swift test` - Run XCTest suite for the SDK (in `sdk/` directory)

**Xcode Workspace:**
- Open `SmartSpectra.xcworkspace` in Xcode for full project development
- Build and run demo apps directly in Xcode on physical iOS devices only (simulator not supported)

**Demo Applications:**
- `samples/demo-app/` - Main demo app with full SDK integration examples
- `samples/smartspectra-trials/` - Trial version demo app
- Build configuration files in `samples/build-configurations/`

**Testing:**
- XCTest framework used for unit tests in `sdk/Tests/SmartSpectraSwiftSDKTests/`
- Physical iOS device required for testing (no simulator support)

## Architecture Overview

**Multi-component iOS Swift SDK for physiological measurement using camera-based computer vision:**

**Core SDK Structure (`sdk/Sources/SmartSpectraSwiftSDK/`):**
- `SmartSpectraSwiftSDK.swift` - Main SDK entry point and shared instance
- `SmartSpectraVitalsProcessor.swift` - Headless vitals processing engine
- `SmartSpectraView.swift` - Complete UI implementation with camera and controls
- `SmartSpectraButtonView.swift` - Individual button component for measurements

**Key Architectural Components:**
- **Authentication System** (`Auth/`): Supports both API key and OAuth authentication
- **Core UI Views** (`Core/`): Tutorial, results, privacy policy, and main button components
- **Camera Management** (`AVCaptureDeviceManager.swift`): Front/back camera switching and control
- **Processing Engine** (`Processing/`): Real-time signal processing and analysis
- **Screening Interface** (`Screening/`): Live measurement UI with recording controls

**Data Processing Pipeline:**
- Native Swift SDK integrates with `PresagePreprocessing.xcframework` (binary framework)
- Protobuf-based data structures for metrics (`metrics.pb.swift`, `point_types.pb.swift`)
- Real-time processing of pulse, breathing, face detection, and vital signs
- Support for both spot measurements (fixed duration) and continuous monitoring

**Key Dependencies:**
- Swift Package Manager with `swift-protobuf` dependency
- Binary xcframework for preprocessing (`Sources/Frameworks/PresagePreprocessing.xcframework`)
- iOS 15.0+ requirement with camera access permissions

**Demo App Integration Pattern:**
- Initialize with API key: `SmartSpectraSwiftSDK.shared.setApiKey("key")`
- Configure mode: `.spot` or `.continuous` measurement types  
- Embed `SmartSpectraView()` for complete UI or use `SmartSpectraVitalsProcessor` headless
- Access metrics through `sdk.metricsBuffer` observable property

**Data Architecture:**
- Protobuf-based `MetricsBuffer` containing pulse, breathing, blood pressure, face detection data
- Real-time mesh points for face landmark tracking via `sdk.meshPoints`
- Confidence scores and stability metrics for quality control
- Binary serialization support for data export/import

**Authentication Requirements:**
- API key from PresageTech developer portal (https://physiology.presagetech.com/)
- OAuth plist configuration file for TestFlight/App Store builds
- Network connectivity required for processing and validation