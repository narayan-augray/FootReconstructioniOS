//
//  SpeechRecognizerEvent.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 06.01.2023.
//

import Foundation

enum SpeechRecognizerEvent {
    case authorizationFailed
    case recognitionFailed
    case recognized(text: String)
}
