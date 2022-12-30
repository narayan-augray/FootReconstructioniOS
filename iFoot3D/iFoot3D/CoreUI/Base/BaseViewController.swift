//
//  BaseViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine
import Toast

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
        
        setupBindings()
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

// MARK: - Private
private extension BaseViewController {
    func setupBindings() {
        viewModel.errorPublisher
            .sink { [weak self] (errorMessage) in
                var toastStyle = ToastStyle()
                toastStyle.backgroundColor = .red
                toastStyle.messageColor = .white
                toastStyle.messageAlignment = .center
                toastStyle.messageFont = Font.sfProTextRegular(17)
                self?.view.makeToast(errorMessage, position: .top, style: toastStyle)
            }
            .store(in: &cancellables)
        
        viewModel.isLoadingPublisher
            .sink { [weak self] (isLoading) in
                isLoading ? self?.view.makeToastActivity(.center) : self?.view.hideToastActivity()
            }
            .store(in: &cancellables)
    }
}
