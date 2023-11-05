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

## Host / User-Agent
User-Agent: ESP3D-TFT-HttpdavServer/1.0 (ESP32S3; Firmware/1.0.0; Platform/RTOS; Embedded; http://www.esp3d.io)
Host: http://192.168.0.1/webdav

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
- 200 if the resource is a file  and the request was successful
- 405 if the resource is a directory 
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
- 200 if the resource is a file and the request was successful
- 404 if the resource does not exist
- 405 if the resource is a directory 
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
- 400 if the resource is / or /sd or /fs

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
- 400 if the resource is / or /sd or /fs

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

if one of the resource is locked, the response will be:
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
		<D:lockdiscovery>
    		<D:activelock>
     			<D:locktype><D:write/></D:locktype>
     			<D:lockscope><D:exclusive/></D:lockscope>
     			<D:depth>infinity</D:depth>
     			<D:owner>John</D:owner>
     			<D:timeout>Second-600</D:timeout>
     			<D:locktoken>
      				<D:href>opaquelocktoken:f81de2ad-7f6d-a1b2-4f3c-00a0c91a9d76</D:href>
     			</D:locktoken>
			 </D:activelock>
         </D:lockdiscovery>
	  </D:prop>
	</D:propstat>
  </D:response>
</D:multistatus>
```

If the resource is a directory, the lock informations will be same for each sub resource, including same lock token.
Note: Because we do not actually support lock method, we will never return lock informations.

The response code is:
- 207 if success
- 404 if the resource does not exist
- 503 if any error accessing the local file system (e.g. access denied)

## COPY method
The COPY method is used to copy the resource identified by the Request-URI to the destination URI.
The copy is allowed only for a file and not for a directory.
Only move /fs to /fs and /sd to /sd

The request has the following headers:
- Destination is the path of the resource to create. to be used instead of the Request-URI if present.
- Overwrite is a boolean that indicates if the destination resource should be overwritten if it already exists. If the header is not present, the default value is false.
- Depth can be 0 or 1 or infinity but we do not support collection copy so this header is ignored 

The necessary headers in response are:
- DAV
- Allow

No content

The response code is:
- 201 if success and file is created
- 204 if success and file is overwritten
- 404 if the resource does not exist
- 413 if the resource is a directory
- 412 if the destination resource already exists and overwrite is false
- 409 if overwrite is true and the destination and source are different types (file or directory)
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file creation
- 507 if the file is too big for the targeted file system
- 400 if source or destination is / or /sd or /fs, or try to /sd to /fs or /fs to /sd

## MOVE method
The MOVE method is used to move the resource identified by the Request-URI to the destination URI.
There is no merge of the destination resource and the source resource but the destination resource is overwritten/replaced if it already exists.
Only move /fs to /fs and /sd to /sd

The request has the following headers:
- Destination is the path of the resource to create. to be used instead of the Request-URI if present.
- Overwrite is a boolean that indicates if the destination resource should be overwritten if it already exists. If the header is not present, the default value is false.

The necessary headers in response are:
- DAV
- Allow

No content

The response code is:
- 201 if success and file is created
- 204 if success and file is overwritten
- 404 if the resource does not exist
- 412 if the destination resource already exists and overwrite is false
- 409 if overwrite is true and the destination and source are different types (file or directory)
- 503 if any error accessing the local file system (e.g. access denied)
- 500 if any error during the file creation
- 400 if source or destination is / or /sd or /fs, or try to move /sd to /fs or /fs to /sd

## PROPPATCH method

The PROPPATCH method is used to set and/or remove properties defined on the resource identified by the Request-URI.
The request has the following headers:
- Content-Type is the type of the request body. It can be text/xml or application/xml or application/x-www-form-urlencoded or multipart/form-data but we only support text/xml.
- Content-Length is the size of the request body.

The request body is an xml document containing the properties to set or remove.
The following xml document will set the properties Win32CreationTime, Win32LastAccessTime, Win32LastModifiedTime and Win32FileAttributes. which are not standard properties but are used by windows to display the file properties.
```xml
<?xml version="1.0" encoding="utf-8" ?>
<D:propertyupdate xmlns:D="DAV:" xmlns:Z="urn:schemas-microsoft-com:">
	<D:set>
		<D:prop>
			<Z:Win32CreationTime>Fri, 12 Aug 2022 07:03:25 GMT</Z:Win32CreationTime>
			<Z:Win32LastAccessTime>Sat, 28 Oct 2023 13:59:02 GMT</Z:Win32LastAccessTime>
			<Z:Win32LastModifiedTime>Fri, 12 Aug 2022 07:03:25 GMT</Z:Win32LastModifiedTime>
			<Z:Win32FileAttributes>00000000</Z:Win32FileAttributes>
		</D:prop>
	</D:set>
</D:propertyupdate>
```
TBD: We should to analyse the request body and set the properties accordingly.
But currently we will ignore the request.

The reponse will be the standard response for PROPFIND method for the resource.
Changing the properties should be ignored if the format is not WebDav standard.

TBD: We should to analyse the request body and set the properties accordingly even if the format is not WebDav standard.



## LOCK method
The LOCK method is used to take out a lock of any access type on the resource identified by the Request-URI.

The request has the following headers:
- Timeout is the time in seconds that the lock will be valid. If the header is not present, the default value is infinite.
- Content-Type is the type of the request body. It can be text/xml or application/xml.
- Content-Length is the size of the request body.

Content of the request body is:
```xml

<?xml version="1.0" encoding="utf-8" ?>
<D:lockinfo xmlns:D="DAV:">
	<D:lockscope>
		<D:exclusive/>
	</D:lockscope>
	<D:locktype>
		<D:write/>
	</D:locktype>
	<D:owner>
		<D:href>DESKTOP-LUCTW\luc</D:href>
	</D:owner>
</D:lockinfo>
```
The response need a token that will be used to access and unlock the resource.
This token should be unique for each lock and the format is opaquelocktoken:<UUID>.

The response has the following headers:
- Lock-Token is the token that will be used to access and unlock the resource.
- Content-Type is the type of the response body. It can be text/xml or application/xml.
- Content-Length is the size of the response body.


```xml
<?xml version="1.0" encoding="utf-8"?>
<D:prop xmlns:D="DAV:">
	<D:lockdiscovery>
		<D:activelock>
			<D:locktoken>
				<D:href>opaquelocktoken:f81de2df-7e8d-4fab-81bb-4b268c1323a7</D:href>
			</D:locktoken>
		</D:activelock>
	</D:lockdiscovery>
</D:prop>
```
In our case we will generate a random UUID and return it as the token.


The response code is:
- 200 if success
- 400 if source or destination is / or /sd or /fs, or try to move /sd to /fs or /fs to /sd
- 404 if the resource does not exist
- 423 if the resource is already locked (because we do not support lock method, we will never return this code)
- 412 if the token is invalid (because we do not support lock method, we will never return this code)
- 403 if authorization is invalid (because we do not support lock method, we will never return this code)
- 503 if any error accessing the local file system (e.g. access denied)

## UNLOCK method
The UNLOCK method is used to remove the lock identified by the Lock-Token header.
The request has the following headers:
- Lock-Token is the token that will be used to access and unlock the resource.

The necessary headers in response are:
- DAV
- Allow

No content

The response code is:
- 204 if success
- 400 if source or destination is / or /sd or /fs, or try to move /sd to /fs or /fs to /sd
- 404 if the resource does not exist
- 412 if the token is invalid (because we do not support lock method, we will never return this code)
- 423 if the resource is not locked (because we do not support lock method, we will never return this code)
- 503 if any error accessing the local file system (e.g. access denied)

