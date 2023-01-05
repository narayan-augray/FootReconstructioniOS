//
//  SoleCaptureModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import Combine

enum SoleCaptureTransition: Transition {
    
}

final class SoleCaptureModuleBuilder {
    class func build(
        container: AppContainer,
        outputs: [CaptureProcessedOutput]
    ) -> Module<SoleCaptureTransition, UIViewController> {
        let viewModel = SoleCaptureViewModel(outputs: outputs,
                                             arSessionManager: container.arSessionManager)
        let viewController = SoleCaptureViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
