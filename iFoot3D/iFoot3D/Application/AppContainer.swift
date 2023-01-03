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
}

final class AppContainerImpl: AppContainer {
    // MARK: - Services
    public lazy var arSessionManager: ARSessionManager = setupARSessionManager()
    public lazy var captureService: CaptureService = setupCaptureService()
}

// MARK: - Setup
private extension AppContainerImpl {
    func setupARSessionManager() -> ARSessionManager {
        return ARSessionManagerImpl()
    }
    
    func setupCaptureService() -> CaptureService {
        return CaptureServiceImpl()
    }
}
