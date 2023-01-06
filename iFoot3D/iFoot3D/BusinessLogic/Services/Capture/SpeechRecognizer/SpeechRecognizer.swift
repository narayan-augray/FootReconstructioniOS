//
//  SpeechRecognizer.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import Foundation
import Speech
import Combine

protocol SpeechRecognizer {
    // MARK: - Properties
    var eventPublisher: AnyPublisher<SpeechRecognizerEvent, Never> { get }
    
    // MARK: - Funcs
    func startRecognition()
    func stopRecognition()
}

final class SpeechRecognizerImpl: SpeechRecognizer {
    // MARK: - Properties
    private let audioEngine: AVAudioEngine
    private let speechRecognizer: SFSpeechRecognizer?
    private var recognitionRequest: SFSpeechAudioBufferRecognitionRequest?
    private var recognitionTask: SFSpeechRecognitionTask?
    private var isRecognizerActive: Bool
    private var previousText: String?
    
    // MARK: - Publishers
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<SpeechRecognizerEvent, Never>()
    
    // MARK: - Init
    init() {
        self.audioEngine = AVAudioEngine()
        self.speechRecognizer = SFSpeechRecognizer(locale: .init(identifier: Constant.languageLocale))
        self.isRecognizerActive = false
    }
}

// MARK: - Controls
extension SpeechRecognizerImpl {
    func startRecognition() {
        SFSpeechRecognizer.requestAuthorization { [weak self] (status) in
            switch status {
            case .authorized:
                do {
                    try self?.start()
                } catch {
                    self?.eventSubject.send(.recognitionFailed)
                }
                
            default:
                self?.eventSubject.send(.authorizationFailed)
            }
        }
    }
    
    func stopRecognition() {
        guard isRecognizerActive else { return }
        isRecognizerActive = false
        audioEngine.inputNode.removeTap(onBus: 0)
        audioEngine.reset()
        audioEngine.stop()
        recognitionRequest?.endAudio()
        recognitionTask?.cancel()
        recognitionTask = nil
        recognitionRequest = nil
    }
}

// MARK: - Private
private extension SpeechRecognizerImpl {
    func start() throws {
        guard !isRecognizerActive else { return }
        
        // Cancel previous recognition task
        recognitionTask?.cancel()
        recognitionTask = nil
        
        // Audio input node
        let inputNode = audioEngine.inputNode
        
        // Create and configure speech recognition request
        recognitionRequest = SFSpeechAudioBufferRecognitionRequest()
        guard let recognitionRequest = recognitionRequest else {
            fatalError("Unable to create a SFSpeechAudioBufferRecognitionRequest object")
        }
        recognitionRequest.shouldReportPartialResults = true
        recognitionRequest.requiresOnDeviceRecognition = false
        
        // Create a recognition task for the speech recognition session with reference in order to cancel
        recognitionTask = speechRecognizer?.recognitionTask(with: recognitionRequest) { [weak self] (result, error) in
            self?.handleRecognitionResults(result: result, error: error)
        }
        
        // Configure the microphone input
        let recordingFormat = inputNode.outputFormat(forBus: 0)
        inputNode.removeTap(onBus: 0)
        inputNode.installTap(
            onBus: 0,
            bufferSize: Constant.bufferSize,
            format: recordingFormat
        ) { [weak self] (buffer, time) in
            self?.recognitionRequest?.append(buffer)
        }
        
        // Prepare and start recognition
        audioEngine.prepare()
        try audioEngine.start()
        isRecognizerActive = true
    }
    
    func handleRecognitionResults(result: SFSpeechRecognitionResult?, error: Error?) {
        var isFinal = false
        
        if let result = result {
            let textResult = result.bestTranscription.formattedString.lowercased()
            
            var outputText = textResult
            if let previousText = previousText {
                outputText = textResult.replacingOccurrences(of: previousText, with: "")
            }
            outputText = outputText.trimmingCharacters(in: .whitespaces)
            
            previousText = textResult
            
            eventSubject.send(.recognized(text: outputText))
            
            isFinal = result.isFinal
        }
        
        // Stop recognizing if there is a problem.
        if error != nil || isFinal {
            audioEngine.stop()
            audioEngine.inputNode.removeTap(onBus: 0)
            
            recognitionRequest = nil
            recognitionTask = nil
            
            eventSubject.send(.recognitionFailed)
        }
    }
}

// MARK: - Constants
private struct Constant {
    static let languageLocale: String = "en-US"
    static let bufferSize: UInt32 = 1024
}
