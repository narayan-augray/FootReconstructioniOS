//
//  UIView+Constraints.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit

extension UIView {
    func addSubview(_ other: UIView, constraints: [NSLayoutConstraint]) {
        addSubview(other)
        other.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate(constraints)
    }

    func addSubview(_ other: UIView, withEdgeInsets edgeInsets: UIEdgeInsets, safeArea: Bool = false) {
        if safeArea {
            addSubview(other, constraints: [
                other.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.leadingAnchor, constant: edgeInsets.left),
                other.topAnchor.constraint(equalTo: safeAreaLayoutGuide.topAnchor, constant: edgeInsets.top),
                other.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.trailingAnchor, constant: -edgeInsets.right),
                other.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor, constant: -edgeInsets.bottom)
            ])
        } else {
            addSubview(other, constraints: [
                other.leadingAnchor.constraint(equalTo: leadingAnchor, constant: edgeInsets.left),
                other.topAnchor.constraint(equalTo: topAnchor, constant: edgeInsets.top),
                other.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -edgeInsets.right),
                other.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -edgeInsets.bottom)
            ])
        }
    }

    func addSubviewToCenter(_ other: UIView) {
        addSubview(other, constraints: [
            other.centerYAnchor.constraint(equalTo: centerYAnchor),
            other.centerXAnchor.constraint(equalTo: centerXAnchor)
        ])
    }
    
    func addSubviewToCenter(_ other: UIView, width: CGFloat, height: CGFloat) {
        addSubview(other, constraints: [
            other.centerYAnchor.constraint(equalTo: centerYAnchor),
            other.centerXAnchor.constraint(equalTo: centerXAnchor),
            other.heightAnchor.constraint(equalToConstant: height),
            other.widthAnchor.constraint(equalToConstant: width)
        ])
    }
}
