//
//  VoiceCaptureModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import Combine

enum VoiceCaptureTransition: Transition {
    
}

final class VoiceCaptureModuleBuilder {
    class func build(
        container: AppContainer,
        outputs: [CaptureProcessedOutput]
    ) -> Module<VoiceCaptureTransition, UIViewController> {
        let viewModel = VoiceCaptureViewModel(outputs: outputs,
                                              arSessionManager: container.arSessionManager)
        let viewController = VoiceCaptureViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
