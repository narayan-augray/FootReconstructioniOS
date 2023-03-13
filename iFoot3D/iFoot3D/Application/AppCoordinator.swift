//
//  AppCoordinator.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

class AppCoordinator: Coordinator {
    // MARK: - Properties
    let container: AppContainer
    var window: UIWindow
    var navigationController: UINavigationController
    var childCoordinators: [Coordinator] = []
    
    // MARK: - Publisher
    private(set) lazy var didFinishPublisher: AnyPublisher<Void, Never> = didFinishSubject.eraseToAnyPublisher()
    private let didFinishSubject = PassthroughSubject<Void, Never>()
    
    // MARK: - Cancellables
    private var cancellables = Set<AnyCancellable>()
    
    // MARK: - Init
    init(
        window: UIWindow,
        container: AppContainer,
        navigationController: UINavigationController = UINavigationController()
    ) {
        self.window = window
        self.container = container
        self.navigationController = navigationController
    }
    
    // MARK: - Start
    func start() {
        window.rootViewController = navigationController
        window.makeKeyAndVisible()
        
        disableIdleTimer()
        
        footCapture()
    }
}

// MARK: - Private
private extension AppCoordinator {
    func footCapture() {
        let module = FootCaptureModuleBuilder.build(container: container)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .instructions(let outputs):
                    self?.instructions(outputs: outputs)
                    
                case .processing(let outputs):
                    self?.processing(outputs: outputs)
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
    
    func instructions(outputs: [CaptureProcessedOutput]) {
        let module = InstructionsModuleBuilder.build(container: container, outputs: outputs)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .capture(let outputs):
                    self?.soleCapture(outputs: outputs)
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
    
    func soleCapture(outputs: [CaptureProcessedOutput]) {
        let module = SoleCaptureModuleBuilder.build(container: container, outputs: outputs)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .process(let outputs):
                    self?.processing(outputs: outputs)
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
    
    func processing(outputs: [CaptureProcessedOutput]) {
        let module = ProcessingModuleBuilder.build(container: container, outputs: outputs)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .success(let outputPath, let outputs, let input):
                    self?.preview(outputPath: outputPath,
                                  outputs: outputs,
                                  input: input)
                    
                case .capture:
                    self?.footCapture()
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController)
    }
    
    func preview(
        outputPath: String,
        outputs: [CaptureProcessedOutput],
        input: ReconstructionInput
    ) {
        let module = PreviewModuleBuilder.build(container: container,
                                                outputPath: outputPath,
                                                outputs: outputs,
                                                input: input)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .close:
                    self?.footCapture()
                    
                case .success:
                    self?.success()
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController)
    }
    
    func success() {
        let module = SuccessModuleBuilder.build(container: container)
        module.transitionPublisher
            .sink { [weak self] (transition) in
                switch transition {
                case .scanAgain:
                    self?.footCapture()
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
}

// MARK: - Helpers
private extension AppCoordinator {
    func disableIdleTimer() {
        UIApplication.shared.isIdleTimerDisabled = true
    }
}
