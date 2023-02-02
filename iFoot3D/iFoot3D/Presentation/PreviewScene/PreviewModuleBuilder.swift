//
//  PreviewModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit
import Combine

enum PreviewTransition: Transition {
    case capture
    case success
}

final class PreviewModuleBuilder {
    class func build(
        container: AppContainer,
        outputs: [CaptureProcessedOutput]
    ) -> Module<PreviewTransition, UIViewController> {
        let viewModel = PreviewViewModel(outputs: outputs)
        let viewController = PreviewViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
