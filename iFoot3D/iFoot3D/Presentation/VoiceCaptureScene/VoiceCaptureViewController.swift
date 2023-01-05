//
//  VoiceCaptureViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

final class VoiceCaptureViewController: BaseViewController<VoiceCaptureViewModel> {
    // MARK: - Views
    private let contentView = VoiceCaptureView()
    
    // MARK: - Lifecycle
    override func loadView() {
        view = contentView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        setupBindings()
    }
}

// MARK: - Private
private extension VoiceCaptureViewController {
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] action in
                switch action {
                }
            }
            .store(in: &cancellables)
    }
}
