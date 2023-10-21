# WebDav Service Description

## Introduction
The protocol is based on HTTP and is designed to allow clients to perform remote Web content authoring operations. The protocol provides facilities to copy, move, delete resources, to query and set properties on resources. The protocol has several features but many are useless for the purpose of this project. The protocol will be followed but using only the features that are needed.

## WebDav version
The version of WebDav that will be used is 1 because we do not need PROPATCH, LOCK and UNLOCK methods.
The DAV header will be set to 1.
The allow header will be set to GET, HEAD, PUT, DELETE, OPTIONS, PROPFIND, MKCOL, COPY, MOVE.

## Time format
The time format used is the local time with the timezone offset:
YYYY-MM-DDThh:mm:ssTZD (e.g. 1997-07-16T19:20:30+01:00)

## GET method
The GET method is used to retrieve information about the resource identified by the Request-URI. And the content of the resource is returned as the response body.
The necessary headers are:
- DAV
- Allow
- Last-Modified
- Content-Length (if the resource is a file)
- Content-Type (if the resource is a file)

The content of the reponse is:
 - the file if the resource is a file
 - empty if the resource is a directory

The response code is:
- 200 if the resource is a file or a directory and the request was successful
- 404 if the resource does not exist
- 500 if any error during the file streaming
- 503 if any error accessing the local file system (e.g. access denied)

## HEAD method
The HEAD method is used to retrieve information about the resource identified by the Request-URI. 
The necessary headers are:
- DAV
- Allow
- Last-Modified
- Content-Length (if the resource is a file)
- Content-Type (if the resource is a file)

Unlike GET method, the HEAD method does not return the content of the resource.

The response code is:
- 200 if the resource is a file or a directory and the request was successful
- 404 if the resource does not exist
- 500 if any error during the file streaming
- 503 if any error accessing the local file system (e.g. access denied)

## OPTIONS method
The OPTIONS method is used to retrieve the communication options for WebDav.
The necessary headers are:
- DAV
- Allow
No content
The response code is always 200.

## DELETE method
The DELETE method is used to delete the resource identified by the Request-URI.
The necessary headers are:
- DAV
- Allow
No content
The response code is:
- 204 if success
- 404 if the resource does not exist
- 503 if any error accessing the local file system (e.g. access denied)




