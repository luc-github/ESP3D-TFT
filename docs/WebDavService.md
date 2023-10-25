# WebDav Service Description

## Introduction
The protocol is based on HTTP and is designed to allow clients to perform remote Web content authoring operations. The protocol provides facilities to copy, move, delete resources, to query and set properties on resources. The protocol has several features but many are useless for the purpose of this project. The protocol will be followed but using only the features that are needed.
The authentication is not implemented yet.
If no namespace is supposed ok for xml tags because there is no possible conflict in webdav requests/responses, windows seems request them so they have been added.
As it is embedded system on local network, the header `Date` is not supported because it make no sense to give 1970 based year date if system date is not initialized.
Date format for last modified and creation date is the GMT time with name of day and month in english, even if the rfc says if can be in this format `1997-12-01T17:42:21-08:00`.

## WebDav version
The version of WebDav that will be used is 1 because we do not need PROPATCH, LOCK and UNLOCK methods.
The DAV header will be set to 1.
The allow header will be set to GET, HEAD, PUT, DELETE, OPTIONS, PROPFIND, MKCOL, COPY, MOVE.

## Time format
The time format used is the local time with the timezone offset:
YYYY-MM-DDThh:mm:ssTZD (e.g. 1997-07-16T19:20:30+01:00)

## GET method
The GET method is used to retrieve information about the resource identified by the Request-URI. And the content of the resource is returned as the response body.
The necessary headers in response are:
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
The necessary headers in response are:
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
The necessary headers in response are:
- DAV
- Allow
No content
The response code is always 200.

## DELETE method
The DELETE method is used to delete the resource identified by the Request-URI.
The necessary headers in response are:
- DAV
- Allow
No content
The response code is:
- 204 if success
- 404 if the resource does not exist
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file/dir deletion

## MKCOL method
The MKCOL method is used to create a new collection resource at the location specified by the Request-URI.
The request has the following headers:   
- Destination is the path of the resource to create. to be used instead of the Request-URI if present.
The necessary headers in response are:
- DAV
- Allow
No content
The response code is:
- 201 if success
- 409 if the resource already exists
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the dir creation

## PUT method
The PUT method is used to create a new non-collection resource at the location specified by the Request-URI.
The request has the following headers:   
- Content-Length is the size  of the file.
- Content-Type is the type of the file.

no Overwrite header because we always overwrite the file.

the request content is the content of the resource to create.
The necessary headers in response are:
- DAV
- Allow
- Last-Modified
No content
The response code is:
- 201 if success and file is created
- 204 if success and file is overwritten
- 412 if the resource is a directory
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file creation
- 507 if the file is too big for the targeted file system

## PROPFIND method  
The PROPFIND method retrieves properties defined on the resource identified by the Request-URI, if the Request-URI is a collection it returns the properties of the collection and the properties of the members of the collection.
The request has the following headers:
- Depth can be 0 or 1 or infinity but we do not support infinity so instead we use 1 and that will be returned as the depth in the PROPFIND response
- Content-Type is the type of the request body. It can be text/xml or application/xml or application/x-www-form-urlencoded or multipart/form-data but we only support text/xml.
this content can be ignored because we will always return the same content type.
- Content-Length is the size of the request body.

The request body can be ignored because we will always return the same content

The necessary headers in response are:
- DAV
- Allow
- Content-Type is text/xml
- Connection: close
- Content-Length is the size of the response body but because it is not possible to know the size of the response body before generating it because we do not know the number of resources in the directory, we will use chunked transfer encoding. so this header will not be present in the response.
 - depth=1 will only be added in case of a request of depth=infinity

The response body is an xml document containing the properties of the resource.
```xml
<?xml version="1.0" encoding="utf-8"?>
<D:multistatus xmlns:D="DAV:">
  <D:response xmlns:esp3d="DAV:">
	<D:href>/webdav</D:href>
	<D:propstat>
	  <D:status>HTTP/1.1 200 OK</D:status>
	  <D:prop>
		<esp3d:getlastmodified>Wed, 25 Oct 2023 04:44:55 GMT</esp3d:getlastmodified>
		<esp3d:creationdate>Wed, 25 Oct 2023 04:44:55 GMT</esp3d:creationdate>
		<D:resourcetype>
		  <D:collection/>
		</D:resourcetype>
		<esp3d:displayname>/</esp3d:displayname>
	  </D:prop>
	</D:propstat>
  </D:response>
  <D:response xmlns:esp3d="DAV:">
	<D:href>/webdav/fs</D:href>
	<D:propstat>
	  <D:status>HTTP/1.1 200 OK</D:status>
	  <D:prop>
		<esp3d:getlastmodified>Wed, 31 Dec 1969 23:59:59 GMT</esp3d:getlastmodified>
		<esp3d:creationdate>Thu, 01 Jan 1970 00:00:00 GMT</esp3d:creationdate>
		<D:resourcetype>
		  <D:collection/>
		</D:resourcetype>
		<esp3d:displayname>fs</esp3d:displayname>
	  </D:prop>
	</D:propstat>
  </D:response>
  <D:response xmlns:esp3d="DAV:">
	<D:href>/webdav/sd</D:href>
	<D:propstat>
	  <D:status>HTTP/1.1 200 OK</D:status>
	  <D:prop>
		<esp3d:getlastmodified>Thu, 01 Jan 1970 00:00:00 GMT</esp3d:getlastmodified>
		<esp3d:creationdate>Thu, 01 Jan 1970 00:00:00 GMT</esp3d:creationdate>
		<D:resourcetype>
		  <D:collection/>
		</D:resourcetype>
		<esp3d:displayname>sd</esp3d:displayname>
	  </D:prop>
	</D:propstat>
  </D:response>
</D:multistatus>

```

The response code is:
- 207 if success
- 404 if the resource does not exist
- 503 if any error accessing the local file system (e.g. access denied)

## COPY method
The COPY method is used to copy the resource identified by the Request-URI to the destination URI.
The copy is allowed only for a file and not for a directory.

The request has the following headers:
- Destination is the path of the resource to create. to be used instead of the Request-URI if present.
- Overwrite is a boolean that indicates if the destination resource should be overwritten if it already exists. If the header is not present, the default value is false.

The necessary headers in response are:
- DAV
- Allow
- Last-Modified

No content

The response code is:
- 201 if success and file is created
- 204 if success and file is overwritten
- 404 if the resource does not exist
- 413 if the resource is a directory
- 412 if the destination resource already exists and overwrite is false
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file creation
- 507 if the file is too big for the targeted file system

## MOVE method
The MOVE method is used to move the resource identified by the Request-URI to the destination URI.
There is no merge of the destination resource and the source resource but the destination resource is overwritten/replaced if it already exists.

The request has the following headers:
- Destination is the path of the resource to create. to be used instead of the Request-URI if present.
- Overwrite is a boolean that indicates if the destination resource should be overwritten if it already exists. If the header is not present, the default value is false.

The necessary headers in response are:
- DAV
- Allow
- Last-Modified

No content

The response code is:
- 201 if success and file is created
- 204 if success and file is overwritten
- 404 if the resource does not exist
- 412 if the destination resource already exists and overwrite is false
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file creation








