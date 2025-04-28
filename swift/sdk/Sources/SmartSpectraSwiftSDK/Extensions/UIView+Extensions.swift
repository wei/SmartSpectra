//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 7/27/23.
//


import Foundation
import UIKit


extension UIView {
    var viewController: UIViewController? {
        var responder: UIResponder? = self
        while let nextResponder = responder?.next {
            if let viewController = nextResponder as? UIViewController {
                return viewController
            }
            responder = nextResponder
        }
        return nil
    }
    
    private struct AssociatedObjectKeys {
        static var tapGestureRecognizer = "TapGestureRecognizer"
        static var tapDelayTimer = "TapDelayTimer"
    }
    
    private typealias Action = () -> Void
    private var tapGestureRecognizerAction: Action? {
        get {
            return objc_getAssociatedObject(self, &AssociatedObjectKeys.tapGestureRecognizer) as? Action
        }
        set {
            objc_setAssociatedObject(
                self,
                &AssociatedObjectKeys.tapGestureRecognizer,
                newValue,
                .OBJC_ASSOCIATION_RETAIN
            )
        }
    }
    
    private var tapDelayTimer: Timer? {
        get {
            return objc_getAssociatedObject(self, &AssociatedObjectKeys.tapDelayTimer) as? Timer
        }
        set {
            objc_setAssociatedObject(
                self,
                &AssociatedObjectKeys.tapDelayTimer,
                newValue,
                .OBJC_ASSOCIATION_RETAIN
            )
        }
    }
    
    public func addTapGestureRecognizer(numberOfTaps: Int = 1, action: (() -> Void)?) {
        isUserInteractionEnabled = true
        tapGestureRecognizerAction = action
        
        // Remove any existing tap gesture recognizers
        removeTapGestureRecognizers()
        
        let tapGestureRecognizer = UITapGestureRecognizer(
            target: self,
            action: #selector(handleTapGesture)
        )
        tapGestureRecognizer.numberOfTapsRequired = numberOfTaps
        addGestureRecognizer(tapGestureRecognizer)
    }
    
    private func removeTapGestureRecognizers() {
        gestureRecognizers?
            .filter { $0 is UITapGestureRecognizer }
            .forEach { removeGestureRecognizer($0) }
    }
    
    @objc private func handleTapGesture(sender: UITapGestureRecognizer) {
        if tapDelayTimer == nil {
            tapGestureRecognizerAction?()
            
            tapDelayTimer = Timer.scheduledTimer(
                timeInterval: 1.0,
                target: self,
                selector: #selector(resetTapDelayTimer),
                userInfo: nil,
                repeats: false
            )
        }
    }
    
    @objc private func resetTapDelayTimer() {
        tapDelayTimer?.invalidate()
        tapDelayTimer = nil
    }
    
    func animateScaleWithBounce(duration: TimeInterval, scale: CGFloat, damping: CGFloat, velocity: CGFloat) {
        UIView.animate(withDuration: duration, delay: 0, usingSpringWithDamping: damping, initialSpringVelocity: velocity, options: .curveEaseInOut, animations: {
            self.transform = CGAffineTransform(scaleX: scale, y: scale)
        }) { (_) in
            UIView.animate(withDuration: duration, delay: 0, usingSpringWithDamping: damping, initialSpringVelocity: velocity, options: .curveEaseInOut, animations: {
                self.transform = CGAffineTransform.identity
            }) { (_) in
                self.animateScaleWithBounce(duration: duration, scale: scale, damping: damping, velocity: velocity)
            }
        }
    }
}
