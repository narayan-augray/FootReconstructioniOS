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
        
        capture()
    }
}

// MARK: - Private
private extension AppCoordinator {
    func capture() {
        let module = CaptureModuleBuilder.build(container: container)
        module.transitionPublisher
            .sink { [unowned self] (transition) in
                
            }
            .store(in: &cancellables)
        push(module.viewController, animated: false)
    }
}
