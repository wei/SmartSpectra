//
//  AVCaptureDeviceManager.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/4/25.
//

import Foundation
import AVFoundation
import CoreImage
import UIKit

class AVCaptureDeviceManager: NSObject {
    internal static let shared = AVCaptureDeviceManager()
    var captureSession: AVCaptureSession!
    var videoDeviceInput: AVCaptureDeviceInput!
    var photoOutput: AVCapturePhotoOutput!
    let videoDataOutput = AVCaptureVideoDataOutput()
    let videoDataOutputQueue = DispatchQueue(label: "com.presagetech.smartspectra.videoDataOutputQueue")
    var previewLayer: AVCaptureVideoPreviewLayer?
    private var sdkConfig = SmartSpectraSdkConfig.shared

    private override init () {
        super.init()
        captureSession = AVCaptureSession()
        setupCamera()
    }

    private func setupCamera() {
        guard AVCaptureDevice.default(for: .video) != nil else {
            return
        }
        switch AVCaptureDevice.authorizationStatus(for: .video) {
        case .authorized:
            if let cameraDevice = AVCaptureDevice.default(.builtInWideAngleCamera, for: .video, position: sdkConfig.cameraPosition) {
                do {
                    videoDeviceInput = try AVCaptureDeviceInput(device: cameraDevice)
                    if captureSession.canAddInput(videoDeviceInput) {
                        captureSession.addInput(videoDeviceInput)
                    }

                    photoOutput = AVCapturePhotoOutput()
                    if captureSession.canAddOutput(photoOutput) {
                        captureSession.addOutput(photoOutput)
                    }
                } catch {
                    Logger.log("Error setting up front camera input: \(error.localizedDescription)")
                }
            } else {
                Logger.log("Front camera not available.")
            }
        case .notDetermined:
            AVCaptureDevice.requestAccess(for: .video) { [weak self] granted in
                guard let self = self else {
                    Logger.log("Unable to gain camera access.")
                    return
                }
                if granted {
                    if let cameraDevice = AVCaptureDevice.default(.builtInWideAngleCamera, for: .video, position: sdkConfig.cameraPosition) {
                        do {
                            videoDeviceInput = try AVCaptureDeviceInput(device: cameraDevice)
                            if captureSession.canAddInput(videoDeviceInput) {
                                captureSession.addInput(videoDeviceInput)
                            }

                            photoOutput = AVCapturePhotoOutput()
                            if captureSession.canAddOutput(photoOutput) {
                                captureSession.addOutput(photoOutput)
                            }
                        } catch {
                            Logger.log("Error setting up front camera input: \(error.localizedDescription)")
                        }
                    } else {
                        Logger.log("Requested Camera not available.")
                    }
                } else {
                    Logger.log("Camera access was not granted.")
                }
            }
        case .restricted, .denied:
            Logger.log("Camera access is restricted or denied.")
        @unknown default:
            Logger.log("Camera access is restricted or denied.")
        }
    }

    internal func lockCameraSettings() {
        guard let device = videoDeviceInput?.device else {
            Logger.log("Error: videoDeviceInput is nil")
            return
        }
        do {
            try device.lockForConfiguration()

            if device.isFocusModeSupported(.locked) {
                device.focusMode = .locked
            }

            if device.isWhiteBalanceModeSupported(.locked) {
                device.whiteBalanceMode = .locked
            }

            if device.isExposureModeSupported(.locked) {
                device.exposureMode = .locked
            }

            device.unlockForConfiguration()
        } catch {
            Logger.log("Error locking camera settings: \(error.localizedDescription)")
        }
    }

    internal func unlockCameraSettings() {
        guard let device = videoDeviceInput?.device else {
            Logger.log("Error: videoDeviceInput is nil")
            return
        }
        do {
            try device.lockForConfiguration()

            if device.isFocusModeSupported(.continuousAutoFocus) {
                device.focusMode = .continuousAutoFocus
            } else if device.isFocusModeSupported(.autoFocus) {
                device.focusMode = .autoFocus
            }

            if device.isWhiteBalanceModeSupported(.continuousAutoWhiteBalance) {
                device.whiteBalanceMode = .continuousAutoWhiteBalance
            } else if device.isWhiteBalanceModeSupported(.autoWhiteBalance)  {
                device.whiteBalanceMode = .autoWhiteBalance
            }

            if device.isExposureModeSupported(.continuousAutoExposure) {
                device.exposureMode = .continuousAutoExposure
            } else if device.isExposureModeSupported(.autoExpose) {
                device.exposureMode = .autoExpose
            }

            device.unlockForConfiguration()
        } catch {
            Logger.log("Error unlocking camera settings: \(error.localizedDescription)")
        }
    }
}

extension AVCaptureDeviceManager: AVCaptureVideoDataOutputSampleBufferDelegate { //TODO: evaluate if this is being used

    public func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        // Process the light data here
        guard let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else {
            return
        }

        let ciImage = CIImage(cvPixelBuffer: imageBuffer)
        let context = CIContext()

        if let cgImage = context.createCGImage(ciImage, from: ciImage.extent) {
            let image = UIImage(cgImage: cgImage)

            // Process the image or extract light intensity
            if let lightIntensity = image.averageBrightness() {
                print("Light intensity: \(lightIntensity)")
            }
        }
    }

}
