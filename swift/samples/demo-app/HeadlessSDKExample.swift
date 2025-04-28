//
//  HeadlessSDKExample.swift
//  demo-app
//
//  Created by Ashraful Islam on 3/5/25.
//


import SwiftUI
import SmartSpectraSwiftSDK

@available(iOS 16.0, *)
struct HeadlessSDKExample: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var isVitalMonitoringEnabled: Bool = false

    init() {
        // (Required), If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // (Required), Deprecated. set apiKey. API key from https://physiology.presagetech.com. Leave default if you want to use oauth. Oauth overrides api key
        /*
         * assuming you already set it in ContentView.swift, you don't need to set it here again
         */
//        let apiKey = "YOUR_API_KEY_HERE"

//        sdk.setApiKey(apiKey)
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
        .onDisappear {
            stopVitalsMonitoring()
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
