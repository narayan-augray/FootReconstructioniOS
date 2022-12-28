//
//  Coordinator.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

protocol Coordinator: AnyObject {
    // MARK: - Properties
    var childCoordinators: [Coordinator] { get set }
    var navigationController: UINavigationController { get set }
    var didFinishPublisher: AnyPublisher<Void, Never> { get }
    
    // MARK: - Funcs
    func start()
}

// MARK: - Navigation
extension Coordinator {
    func addChild(coordinator: Coordinator) {
        childCoordinators.append(coordinator)
    }
    
    func removeChild(coordinator: Coordinator) {
        childCoordinators = childCoordinators.filter { $0 !== coordinator }
    }
    
    func setRoot(_ viewController: UIViewController, animated: Bool = true) {
        navigationController.setViewControllers([viewController],
                                                animated: animated)
    }
    
    func setRoot(_ viewControllers: [UIViewController], animated: Bool = true) {
        navigationController.setViewControllers(viewControllers,
                                                animated: animated)
    }
    
    func push(_ viewController: UIViewController, animated: Bool = true) {
        navigationController.pushViewController(viewController,
                                                animated: animated)
    }
    
    func pop(animated: Bool = true) {
        navigationController.popViewController(animated: animated)
    }
    
    func present(
        _ viewController: UIViewController,
        animated: Bool = true,
        completion: (() -> Void)? = nil
    ) {
        navigationController.controllerForPresentation.present(viewController,
                                                               animated: animated,
                                                               completion: completion)
    }
    
    func dismiss(
        animated: Bool = true,
        completion: (() -> Void)? = nil
    ) {
        navigationController.controllerForPresentation.dismiss(animated: true,
                                                               completion: completion)
    }
}
