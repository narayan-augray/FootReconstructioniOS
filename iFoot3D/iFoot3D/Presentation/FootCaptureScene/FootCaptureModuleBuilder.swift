//
//  FootCaptureModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

enum FootCaptureTransition: Transition {
    case instructions(outputs: [CaptureProcessedOutput])
}

final class FootCaptureModuleBuilder {
    class func build(container: AppContainer) -> Module<FootCaptureTransition, UIViewController> {
        let viewModel = FootCaptureViewModel(arSessionManager: container.arSessionManager,
                                             captureService: container.captureService,
                                             captureOutputManager: container.captureOutputManager)
        let viewController = FootCaptureViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
