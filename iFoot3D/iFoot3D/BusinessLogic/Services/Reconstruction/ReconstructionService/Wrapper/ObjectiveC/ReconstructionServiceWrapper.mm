//
//  ReconstructionServiceWrapper.mm
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

#import <Foundation/Foundation.h>
#import <opencv2/core.hpp>
#import "ReconstructionServiceWrapper.h"
#import "io.h"
#import "reconstruction.h"

@implementation ReconstructionServiceWrapper
// MARK: - Init
- (instancetype) init {
    self = [super init];
    return self;
}

// MARK: - Reconstruction
- (void) reconstruct: (NSArray<NSArray<NSString *> *> *) rightSidePaths
       leftSidePaths: (NSArray<NSArray<NSString *> *> *) leftSide
           solePaths: (NSArray<NSArray<NSString *> *> *) sole
          outputPath: (NSString *) output {
    std::vector<std::vector<std::string>> rightPaths = [self getPaths:rightSidePaths];
    std::vector<std::vector<std::string>> leftPaths = [self getPaths:leftSide];
    std::vector<std::vector<std::string>> solePaths = [self getPaths:sole];
    
    std::vector<std::vector<std::vector<cv::Mat>>> input = ifoot3d::readMultipleInputData(rightPaths,
                                                                                          leftPaths,
                                                                                          solePaths);
    // call reconstruct leg function
}

// MARK: - Helpers
- (std::vector<std::vector<std::string>>) getPaths: (NSArray<NSArray<NSString *> *> *) paths {
    std::vector<std::vector<std::string>> output = {};
    
    for (NSArray<NSString *> * path in paths) {
        std::vector<std::string> outputPaths = {};
        
        for (NSString * string in path) {
            std::string path = [string cStringUsingEncoding: NSUTF8StringEncoding];
            outputPaths.push_back(path);
        }
        
        output.push_back(outputPaths);
    }
    
    return output;
}

@end
