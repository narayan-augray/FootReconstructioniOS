//
//  SuccessModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit
import Combine

enum SuccessTransition: Transition {
    case back
}

final class SuccessModuleBuilder {
    class func build(container: AppContainer) -> Module<SuccessTransition, UIViewController> {
        let viewModel = SuccessViewModel()
        let viewController = SuccessViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
