//
//  ProcessingModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit
import Combine

enum ProcessingTransition: Transition {
    case success(outputs: [CaptureProcessedOutput])
}

final class ProcessingModuleBuilder {
    class func build(
        container: AppContainer,
        outputs: [CaptureProcessedOutput]
    ) -> Module<ProcessingTransition, UIViewController> {
        let viewModel = ProcessingViewModel(outputs: outputs)
        let viewController = ProcessingViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
