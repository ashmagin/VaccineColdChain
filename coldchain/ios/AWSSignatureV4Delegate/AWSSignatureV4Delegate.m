//
//  AWSSignatureV4Delegate.m
//  coldchain
//
//  Created by Andrei Malashanka on 11.09.21.
//

#import "AWSSignatureV4Delegate.h"

@interface AWSSignatureV4Delegate()
- (NSString *)doubleEncode:(nonnull NSString *)path;
@end


@implementation AWSSignatureV4Delegate
{
  AWSCognitoCredentialsProvider *credentialsProvider;
  NSString *serviceName;
}

- (instancetype _Nonnull)initWithCredentialsProvider:(AWSCognitoCredentialsProvider*)provider serviceName:(NSString*) service
{
  if (self = [super init])
  {
    self->credentialsProvider = provider;
    self->serviceName = service;
  }
  return self;
}

- (NSString *)doubleEncode:(nonnull NSString *)path
{
  NSCharacterSet *cs = [NSCharacterSet URLPathAllowedCharacterSet];
  NSString *firstPass = [path stringByAddingPercentEncodingWithAllowedCharacters:cs];
  if (firstPass == nil)
  {
    return nil;
  }
  NSString *secondPass = [firstPass stringByAddingPercentEncodingWithAllowedCharacters:cs];
  return secondPass;
}

- (nonnull NSURL *)offlineStorage:(nonnull MGLOfflineStorage *)storage URLForResourceOfKind:(MGLResourceKind)kind withURL:(nonnull NSURL *)url {
  if ((url.host != NULL) && ([url.host rangeOfString:@"amazonaws.com"].location == NSNotFound))
  {
    /* not an AWS URL */
    return url;
  }
  
  /* URL-encode spaces, etc. */
  NSString *keyPath = [url.path substringFromIndex:1];
  NSCharacterSet *cs = [NSCharacterSet URLPathAllowedCharacterSet];
  NSString *percentEncodedKeyPath = [keyPath stringByAddingPercentEncodingWithAllowedCharacters:cs];
  if (percentEncodedKeyPath == nil)
  {
    NSLog(@"Invalid characters in path '%@'; unsafe to sign", keyPath);
    return url;
  }
  
  /* Get AWS region from URL: *.<region id>.amazonaws.com */
  NSArray<NSString*> *components = [url.host componentsSeparatedByString:@"."];
  NSString *regionName = [components objectAtIndex:components.count-3];
  AWSRegionType region = regionName.aws_regionTypeValue;
  
  /* Prepare endpoint and request headers */
  AWSEndpoint* endpoint = [[AWSEndpoint alloc] initWithRegion:region serviceName:self->serviceName URL:url];
  NSDictionary *requestHeaders = @{
    @"host": endpoint.hostName,
  };
  
  /* sign the URL */
  AWSTask *task = [AWSSignatureV4Signer generateQueryStringForSignatureV4WithCredentialProvider:
                   self->credentialsProvider
                   httpMethod: AWSHTTPMethodGET
                   expireDuration: 60
                   endpoint: endpoint
                   /* workaround for https://github.com/aws-amplify/aws-sdk-ios/issues/3215 */
                   keyPath: [self doubleEncode: percentEncodedKeyPath]
                   requestHeaders: requestHeaders
                   requestParameters: nil
                   signBody: true];
  (void)task.waitUntilFinished;
  
  NSError *error = task.error;
  
  if (error != nil)
  {
    NSLog(@"Error occurred: %@", error);
  }
  
  NSURL *result = task.result;
  if (result != nil)
  {
    NSURLComponents *urlComponents = [[NSURLComponents alloc] initWithURL:result resolvingAgainstBaseURL:false];
    /* re-use the original path; workaround for https://github.com/aws-amplify/aws-sdk-ios/issues/3215 */
    urlComponents.path = url.path;
    
    /* have Mapbox GL fetch the signed URL */
    if (urlComponents.URL != nil)
    {
      return (urlComponents.URL);
    }
  }

  /* fall back to an unsigned URL */
  return url;
}

@end
