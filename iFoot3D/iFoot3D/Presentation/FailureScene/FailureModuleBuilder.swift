//
//  FailureModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 26.05.2023.
//

import UIKit
import Combine

enum FailureTransition: Transition {
    case tryAgain
}

final class FailureModuleBuilder {
    class func build(container: AppContainer) -> Module<FailureTransition, UIViewController> {
        let viewModel = FailureViewModel()
        let viewController = FailureViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
