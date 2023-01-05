//
//  ActivityViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit

class ActivityViewController: UIActivityViewController {
    // MARK: - Handlers
    var onDismiss: (() -> ())?
    
    // MARK: - Lifecycle
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        onDismiss?()
    }
}
