//
//  SmartSpectraButtonViewModel.swift
//
//
//  Created by Ashraful Islam on 8/14/23.
//

import Foundation
import UIKit
import Combine
import SwiftUI
@available(iOS 15.0, *)
/// A custom button with predefined appearance and behavior for SmartSpectra SDK.
final class SmartSpectraButtonViewModel: ObservableObject {
    
    internal let sdk = SmartSpectraSwiftSDK.shared
    public let responseSubject = PassthroughSubject<String, Never>()

    public init() {
        // Empty public initializer
    }
    
    private func showTutorialAndAgreementIfNecessary(completion: (() -> Void)? = nil) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            let walkthroughShown = UserDefaults.standard.bool(forKey: "WalkthroughShown")
            let hasAgreedToTerms = UserDefaults.standard.bool(forKey: "HasAgreedToTerms")
            let hasAgreedToPrivacyPolicy = UserDefaults.standard.bool(forKey: "HasAgreedToPrivacyPolicy")
            
            func showAgreements() {
                if !hasAgreedToTerms {
                    self.presentUserAgreement {
                        showPrivacyPolicy()
                    }
                } else {
                    showPrivacyPolicy()
                }
            }
            
            func showPrivacyPolicy() {
                if !hasAgreedToPrivacyPolicy {
                    self.presentPrivacyPolicy(completion: completion)
                } else {
                    completion?()
                }
            }
            
            if !walkthroughShown {
                self.handleWalkTrough {
                    showAgreements()
                }
            } else {
                showAgreements()
            }
        }
    }


    internal func handleWalkTrough(completion: (() -> Void)? = nil) {
        let tutorialView = TutorialView(onTutorialCompleted: {
            completion?()
        })
        let hostingController = UIHostingController(rootView: tutorialView)
        findViewController()?.present(hostingController, animated: true, completion: nil)
        
        UserDefaults.standard.set(true, forKey: "WalkthroughShown")
    }
    
    private func presentUserAgreement(completion: (() -> Void)? = nil) {
        checkInternetConnectivity { [weak self] isConnected in
            DispatchQueue.main.async {
                if isConnected {
                    let agreementViewController = ViewController.Agreement.Root()
                    agreementViewController.onCompletion = completion
                    let navigationController = UINavigationController(rootViewController: agreementViewController)
                    navigationController.modalPresentationStyle = .fullScreen
                    navigationController.modalTransitionStyle = .coverVertical

                    self?.findViewController()?.present(navigationController, animated: true, completion: nil)
                } else {
                    self?.showNoInternetConnectionAlert()
                }
            }
        }
    }
    
    private func presentPrivacyPolicy(completion: (() -> Void)? = nil) {
        checkInternetConnectivity { [weak self] isConnected in
            DispatchQueue.main.async {
                if isConnected {
                    let privacyPolicyViewController = ViewController.PrivacyPolicy.Root()
                    privacyPolicyViewController.onCompletion = completion
                    let navigationController = UINavigationController(rootViewController: privacyPolicyViewController)
                    navigationController.modalPresentationStyle = .fullScreen
                    navigationController.modalTransitionStyle = .coverVertical

                    self?.findViewController()?.present(navigationController, animated: true, completion: nil)
                } else {
                    self?.showNoInternetConnectionAlert()
                }
            }
        }
    }
    
    private func checkInternetConnectivity(completion: @escaping (Bool) -> Void) {
        guard let url = URL(string: "https://www.google.com") else {
            completion(false)
            return
        }
        let task = URLSession.shared.dataTask(with: URLRequest(url: url)) { _, response, _ in
            if let httpResponse = response as? HTTPURLResponse, httpResponse.statusCode == 200 {
                completion(true)
            } else {
                completion(false)
            }
        }
        task.resume()
    }
    
    private func showNoInternetConnectionAlert() {
        if let rootViewController = findViewController() {
            let alert = UIAlertController(title: "No Internet Connection", message: "Please check your internet connection and try again.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
            rootViewController.present(alert, animated: true, completion: nil)
        }
    }

    func openSafari(withURL urlString: String) {
        guard let url = URL(string: urlString) else {
            return // Invalid URL, handle error or show an alert
        }

        UIApplication.shared.open(url, options: [:], completionHandler: nil)
    }
    
    @objc func showActionSheet() {
        let actionSheet = UIAlertController(title: nil, message: nil, preferredStyle: .actionSheet)

        // Add options
        actionSheet.addAction(UIAlertAction(title: "Show Tutorial", style: .default) { _ in
            UserDefaults.standard.set(false, forKey: "WalkthroughShown")
            self.handleWalkTrough()
        })
        actionSheet.addAction(UIAlertAction(title: "Instructions for Use", style: .default) { _ in
            self.openSafari(withURL: "https://api.physiology.presagetech.com/instructions")
        })
        actionSheet.addAction(UIAlertAction(title: "Terms of Service", style: .default) { _ in
            self.presentUserAgreement()
        })
        actionSheet.addAction(UIAlertAction(title: "Privacy Policy", style: .default) { _ in
            self.presentPrivacyPolicy()
        })

        actionSheet.addAction(UIAlertAction(title: "Contact Us", style: .default) { _ in
            self.openSafari(withURL: "https://api.physiology.presagetech.com/contact")
        })
        

        // Add cancel button with red text color
        let cancelButton = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            // Handle cancellation
        }
        cancelButton.setValue(UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), forKey: "titleTextColor")
        actionSheet.addAction(cancelButton)
        let viewController = self.findViewController()

        // Show action sheet
        viewController?.present(actionSheet, animated: true, completion: nil)
    }
    
    /// Handle SmartSpectra SDK initialization and present the screening page when the button is tapped.
    internal func smartSpectraButtonTapped() {
        
        // show tutorial first time the user taps the button
        showTutorialAndAgreementIfNecessary { [weak self] in
            guard let self = self else { return }
            
            // Add code here to initialize the SmartSpectra SDK
            // Once the SDK is initialized, you can proceed with presenting the screening page.
            if UserDefaults.standard.bool(forKey: "HasAgreedToTerms")  && UserDefaults.standard.bool(forKey: "HasAgreedToPrivacyPolicy") {
                let sPage = SmartSpectra().ScreeningPage(recordButton: Model.Option.Button.Record.init(backgroundColor: UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), titleColor: .white, borderColor: UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), title: "Record"))
                
                // Assuming you have access to the view controller where the button is added
                let viewController = self.findViewController()
                // Check if the current view controller is embedded in a navigation controller
                if let navigationController = viewController?.navigationController {
                    navigationController.pushViewController(sPage, animated: true)
                } else {
                    // If not, present it modally
                    let nav = UINavigationController(rootViewController: sPage)
                    nav.modalPresentationStyle = .overFullScreen
                    viewController?.present(nav, animated: true)
                }
            }
        }
    }

    /// Helper method to find the view controller in the view hierarchy.
    private func findViewController() -> UIViewController? {
        guard let scene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
              let window = scene.windows.first,
              let rootViewController = window.rootViewController else {
            return nil
        }
        return rootViewController
    }

    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}


