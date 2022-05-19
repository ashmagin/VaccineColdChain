//
//  MapboxSignerModule.m
//  coldchain
//
//  Created by Andrei Malashanka on 11.09.21.
//

#import "MapboxSignerModule.h"
#import <Mapbox/Mapbox.h>
#import "AWSSignatureV4Delegate.h"
#import "AppDelegate.h"

@implementation MapboxSignerModule
{
  AWSSignatureV4Delegate *signingDelegate;
  AWSCognitoCredentialsProvider *credentialProvider;
}

RCT_EXPORT_MODULE(MapboxSigner);

RCT_EXPORT_METHOD(setupRequestSigner:(NSString *)identityPoolId
                  errorCallback:(RCTResponseSenderBlock)error
                  successCallback:(RCTResponseSenderBlock)success)
{
  @try
  {
    /* Credential initialization */
    NSString *regionName = [identityPoolId componentsSeparatedByString:@":"][0];
    AWSRegionType region = regionName.aws_regionTypeValue;
    credentialProvider = [[AWSCognitoCredentialsProvider alloc] initWithRegionType:region identityPoolId:identityPoolId];

    /* Assign request signer */
    signingDelegate = [[AWSSignatureV4Delegate alloc] initWithCredentialsProvider:credentialProvider serviceName:@"geo"];
    
    if (signingDelegate != nil)
    {
      /* Update Mapbox UI on the main thread */
      dispatch_async(dispatch_get_main_queue(), ^{
        MGLOfflineStorage.sharedOfflineStorage.delegate = self->signingDelegate;
        success(@[@"Done"]);
      });
    }
    else
    {
      error(@[@"Can't create signing delegate"]);
    }
  }
  @catch(NSException *e)
  {
    NSString *errorText = [[NSString alloc] initWithFormat:@"%@", e.reason];
    error(@[errorText]);
  }
}

@end
