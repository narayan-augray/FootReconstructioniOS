//
//  InstructionCollectionViewCell.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

class InstructionCollectionViewCell: UICollectionViewCell {
    // MARK: - Subviews
    private let imageView = UIImageView()
    private let contentLabel = UILabel()
    
    // MARK: - Init
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialSetup()
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    // MARK: - Setup
    func setupCell(instruction: Instruction) {
        imageView.image = instruction.image
        contentLabel.text = instruction.description
    }
}

// MARK: - Private
private extension InstructionCollectionViewCell {
    func initialSetup() {
        setupLayout()
        setupUI()
    }

    func setupUI() {
        backgroundColor = .clear
        
        imageView.clipsToBounds = true
        imageView.contentMode = .scaleAspectFit
        imageView.layer.cornerRadius = Constant.imageViewCornerRadius
        
        contentLabel.numberOfLines = 0
        contentLabel.textAlignment = .center
        contentLabel.font = Font.sfProTextMedium(Constant.contentLabelFontSize)
    }

    func setupLayout() {
        addSubview(imageView, constraints: [
            imageView.widthAnchor.constraint(equalToConstant: Constant.imageViewHeight),
            imageView.heightAnchor.constraint(equalTo: imageView.widthAnchor),
            imageView.centerXAnchor.constraint(equalTo: centerXAnchor),
            imageView.centerYAnchor.constraint(equalTo: centerYAnchor, constant: -Constant.imageViewCenterYOffset)
        ])
        
        addSubview(contentLabel, constraints: [
            contentLabel.topAnchor.constraint(equalTo: imageView.bottomAnchor,
                                              constant: Constant.contentLabelTopOffser),
            contentLabel.leadingAnchor.constraint(equalTo: leadingAnchor,
                                                  constant: Constant.contentLabelSideOffset),
            contentLabel.trailingAnchor.constraint(equalTo: trailingAnchor,
                                                   constant: -Constant.contentLabelSideOffset)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let imageViewCornerRadius: CGFloat = 10.0
    static let imageViewCenterYOffset: CGFloat = 100.0
    static let imageViewHeight: CGFloat = 350.0
    static let contentLabelFontSize: CGFloat = 19.0
    static let contentLabelTopOffser: CGFloat = 25.0
    static let contentLabelSideOffset: CGFloat = 15.0
}
