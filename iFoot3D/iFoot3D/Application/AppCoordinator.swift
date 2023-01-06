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
        
        footCapture()
    }
}

// MARK: - Private
private extension AppCoordinator {
    func footCapture() {
        let module = FootCaptureModuleBuilder.build(container: container)
        module.transitionPublisher
            .sink { [unowned self] (transition) in
                switch transition {
                case .instructions(let outputs):
                    instructions(outputs: outputs)
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
                case .success(let outputs):
                    self?.success(outputs: outputs)
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
    
    func success(outputs: [CaptureProcessedOutput]) {
        let module = SuccessModuleBuilder.build(container: container, outputs: outputs)
        module.transitionPublisher
            .sink { [unowned self] (transition) in
                switch transition {
                case .back:
                    footCapture()
                }
            }
            .store(in: &cancellables)
        setRoot(module.viewController, animated: true)
    }
}
