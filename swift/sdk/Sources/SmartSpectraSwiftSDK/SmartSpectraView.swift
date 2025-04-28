//
//  SmartSpectraSwiftView.swift
//
//
//  Created by Ashraful Islam on 8/13/24.
//

import Foundation
import SwiftUI
import PresagePreprocessing
import AVFoundation

@available(iOS 15.0, *)
public struct SmartSpectraView: View {

    public init() {
    }

    public var body: some View {
        VStack {
            SmartSpectraButtonView()
            SmartSpectraResultView()
        }
        .edgesIgnoringSafeArea(.all)
    }
}
