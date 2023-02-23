//
//  AppContainer.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation

protocol AppContainer: AnyObject {
    var arSessionManager: ARSessionManager { get }
    var captureService: CaptureService { get }
    var captureOutputManager: CaptureOutputManager { get }
    var speechRecognizer: SpeechRecognizer { get }
    var reconstructionService: ReconstructionService { get }
}

final class AppContainerImpl: AppContainer {
    // MARK: - Services
    lazy var arSessionManager: ARSessionManager = setupARSessionManager()
    lazy var captureService: CaptureService = setupCaptureService()
    lazy var captureOutputManager: CaptureOutputManager = setupCaptureOutputManager()
    lazy var speechRecognizer: SpeechRecognizer = setupSpeechRecognizer()
    lazy var reconstructionService: ReconstructionService = setupReconstructionService()
}

// MARK: - Setup
private extension AppContainerImpl {
    func setupARSessionManager() -> ARSessionManager {
        return ARSessionManagerImpl()
    }
    
    func setupCaptureService() -> CaptureService {
        return CaptureServiceImpl()
    }
    
    func setupCaptureOutputManager() -> CaptureOutputManager {
        let outputSettings = CaptureOutputSettings(
            originalFileName: "original",
            dataTextFileName: "depth_logs",
            calibrationTextFileName: "depth_calibration"
        )
        return CaptureOutputManagerImpl(outputSettings: outputSettings)
    }
    
    func setupSpeechRecognizer() -> SpeechRecognizer {
        return SpeechRecognizerImpl()
    }
    
    func setupReconstructionService() -> ReconstructionService {
        return ReconstructionServiceImpl()
    }
}
