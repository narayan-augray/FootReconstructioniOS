//
//  PreviewModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit
import Combine

enum PreviewTransition: Transition {
    case close
    case success
}

final class PreviewModuleBuilder {
    class func build(
        container: AppContainer,
        modelPath: String
    ) -> Module<PreviewTransition, UIViewController> {
        let viewModel = PreviewViewModel(modelPath: modelPath)
        let viewController = PreviewViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
