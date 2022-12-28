//
//  BaseViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

class BaseViewController<VM: ViewModel>: UIViewController {
    // MARK: - Properties
    var viewModel: VM
    
    // MARK: - Cancellables
    var cancellables = Set<AnyCancellable>()
    
    // MARK: - Init
    init(viewModel: VM) {
        self.viewModel = viewModel
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    // MARK: - Lifecycle
    override func viewDidLoad() {
        super.viewDidLoad()
        viewModel.onViewDidLoad()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        viewModel.onViewWillAppear()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        viewModel.onViewDidAppear()
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        viewModel.onViewWillDisappear()
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        viewModel.onViewDidDisappear()
    }
    
    // MARK: - Deinit
    deinit {
        log.debug(message: "deinit of \(String(describing: self))")
    }
}
