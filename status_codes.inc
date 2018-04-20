// status_codes.inc contains constant definitions for status codes and strings.

// E_HTTP_STATUS contains all standard HTTP status codes
enum E_HTTP_STATUS {
	HTTP_STATUS_CONTINUE            = 100, // RFC 7231, 6.2.1
	HTTP_STATUS_SWITCHING_PROTOCOLS = 101, // RFC 7231, 6.2.2
	HTTP_STATUS_PROCESSING          = 102, // RFC 2518, 10.1

	HTTP_STATUS_OK                  = 200, // RFC 7231, 6.3.1
	HTTP_STATUS_CREATED             = 201, // RFC 7231, 6.3.2
	HTTP_STATUS_ACCEPTED            = 202, // RFC 7231, 6.3.3
	HTTP_STATUS_NON_AUTH_INFO       = 203, // RFC 7231, 6.3.4
	HTTP_STATUS_NO_CONTENT          = 204, // RFC 7231, 6.3.5
	HTTP_STATUS_RESET_CONTENT       = 205, // RFC 7231, 6.3.6
	HTTP_STATUS_PARTIAL_CONTENT     = 206, // RFC 7233, 4.1
	HTTP_STATUS_MULTI_STATUS        = 207, // RFC 4918, 11.1
	HTTP_STATUS_ALREADY_REPORTED    = 208, // RFC 5842, 7.1
	HTTP_STATUS_IMUSED              = 226, // RFC 3229, 10.4.1

	HTTP_STATUS_MULTIPLE_CHOICES    = 300, // RFC 7231, 6.4.1
	HTTP_STATUS_MOVED_PERMANENTLY   = 301, // RFC 7231, 6.4.2
	HTTP_STATUS_FOUND               = 302, // RFC 7231, 6.4.3
	HTTP_STATUS_SEE_OTHER           = 303, // RFC 7231, 6.4.4
	HTTP_STATUS_NOT_MODIFIED        = 304, // RFC 7232, 4.1
	HTTP_STATUS_USE_PROXY           = 305, // RFC 7231, 6.4.5
	HTTP_STATUS_UNUSED              = 306, // RFC 7231, 6.4.6 (Unused)
	HTTP_STATUS_TEMPORARY_REDIRECT  = 307, // RFC 7231, 6.4.7
	HTTP_STATUS_PERMANENT_REDIRECT  = 308, // RFC 7538, 3

	HTTP_STATUS_BAD_REQUEST         = 400, // RFC 7231, 6.5.1
	HTTP_STATUS_UNAUTHORIZED        = 401, // RFC 7235, 3.1
	HTTP_STATUS_PAYMENT_REQUIRED    = 402, // RFC 7231, 6.5.2
	HTTP_STATUS_FORBIDDEN           = 403, // RFC 7231, 6.5.3
	HTTP_STATUS_NOT_FOUND           = 404, // RFC 7231, 6.5.4
	HTTP_STATUS_METHOD_NOT_ALLOWED  = 405, // RFC 7231, 6.5.5
	HTTP_STATUS_NOT_ACCEPTABLE      = 406, // RFC 7231, 6.5.6
	HTTP_STATUS_PROXY_AUTH_REQUIRED = 407, // RFC 7235, 3.2
	HTTP_STATUS_REQUEST_TIMEOUT     = 408, // RFC 7231, 6.5.7
	HTTP_STATUS_CONFLICT            = 409, // RFC 7231, 6.5.8
	HTTP_STATUS_GONE                = 410, // RFC 7231, 6.5.9
	HTTP_STATUS_LENGTH_REQUIRED     = 411, // RFC 7231, 6.5.10
	HTTP_STATUS_PRECONDITION_FAILED = 412, // RFC 7232, 4.2
	HTTP_STATUS_ENTITY_TOO_LARGE    = 413, // RFC 7231, 6.5.11
	HTTP_STATUS_REQUEST_URITOO_LONG = 414, // RFC 7231, 6.5.12
	HTTP_STATUS_INVALID_MEDIA_TYPE  = 415, // RFC 7231, 6.5.13
	HTTP_STATUS_RANGE_NOT_SATISFIED = 416, // RFC 7233, 4.4
	HTTP_STATUS_EXPECTATION_FAILED  = 417, // RFC 7231, 6.5.14
	HTTP_STATUS_TEAPOT              = 418, // RFC 7168, 2.3.3
	HTTP_STATUS_INVALID_ENTITY      = 422, // RFC 4918, 11.2
	HTTP_STATUS_LOCKED              = 423, // RFC 4918, 11.3
	HTTP_STATUS_FAILED_DEPENDENCY   = 424, // RFC 4918, 11.4
	HTTP_STATUS_UPGRADE_REQUIRED    = 426, // RFC 7231, 6.5.15
	HTTP_STATUS_PRECOND_REQUIRED    = 428, // RFC 6585, 3
	HTTP_STATUS_TOO_MANY_REQUESTS   = 429, // RFC 6585, 4
	HTTP_STATUS_HEADER_TOO_LARGE    = 431, // RFC 6585, 5
	HTTP_STATUS_LEGAL_FAILURE       = 451, // RFC 7725, 3

	HTTP_STATUS_SERVER_ERROR        = 500, // RFC 7231, 6.6.1
	HTTP_STATUS_NOT_IMPLEMENTED     = 501, // RFC 7231, 6.6.2
	HTTP_STATUS_BAD_GATEWAY         = 502, // RFC 7231, 6.6.3
	HTTP_STATUS_SERVICE_UNAVAILABLE = 503, // RFC 7231, 6.6.4
	HTTP_STATUS_GATEWAY_TIMEOUT     = 504, // RFC 7231, 6.6.5
	HTTP_STATUS_HTTPV_NOT_SUPPORTED = 505, // RFC 7231, 6.6.6
	HTTP_STATUS_NEGOTIATION_LOOP    = 506, // RFC 2295, 8.1
	HTTP_STATUS_NO_STORAGE          = 507, // RFC 4918, 11.5
	HTTP_STATUS_LOOP_DETECTED       = 508, // RFC 5842, 7.2
	HTTP_STATUS_NOT_EXTENDED        = 510, // RFC 2774, 7
	HTTP_STATUS_AUTH_REQUIRED       = 511, // RFC 6585, 6
}

stock const Continue = "Continue";
stock const SwitchingProtocols = "Switching Protocols";
stock const Processing = "Processing";

stock const Ok = "OK";
stock const Created = "Created";
stock const Accepted = "Accepted";
stock const NonAuthInfo = "Non-Authoritative Information";
stock const NoContent = "No Content";
stock const ResetContent = "Reset Content";
stock const PartialContent = "Partial Content";
stock const MultiStatus = "Multi-Status";
stock const AlreadyReported = "Already Reported";
stock const Imused = "IM Used";

stock const MultipleChoices = "Multiple Choices";
stock const MovedPermanently = "Moved Permanently";
stock const Found = "Found";
stock const SeeOther = "See Other";
stock const NotModified = "Not Modified";
stock const UseProxy = "Use Proxy";
stock const Unused = "";
stock const TemporaryRedirect = "Temporary Redirect";
stock const PermanentRedirect = "Permanent Redirect";

stock const BadRequest = "Bad Request";
stock const Unauthorized = "Unauthorized";
stock const PaymentRequired = "Payment Required";
stock const Forbidden = "Forbidden";
stock const NotFound = "Not Found";
stock const MethodNotAllowed = "Method Not Allowed";
stock const NotAcceptable = "Not Acceptable";
stock const ProxyAuthRequired = "Proxy Authentication Required";
stock const RequestTimeout = "Request Timeout";
stock const Conflict = "Conflict";
stock const Gone = "Gone";
stock const LengthRequired = "Length Required";
stock const PreconditionFailed = "Precondition Failed";
stock const EntityTooLarge = "Request Entity Too Large";
stock const RequestUritooLong = "Request URI Too Long";
stock const InvalidMediaType = "Unsupported Media Type";
stock const RangeNotSatisfied = "Requested Range Not Satisfiable";
stock const ExpectationFailed = "Expectation Failed";
stock const Teapot = "I'm a teapot";
stock const InvalidEntity = "Unprocessable Entity";
stock const Locked = "Locked";
stock const FailedDependency = "Failed Dependency";
stock const UpgradeRequired = "Upgrade Required";
stock const PrecondRequired = "Precondition Required";
stock const TooManyRequests = "Too Many Requests";
stock const HeaderTooLarge = "Request Header Fields Too Large";
stock const LegalFailure = "Unavailable For Legal Reasons";

stock const ServerError = "Internal Server Error";
stock const NotImplemented = "Not Implemented";
stock const BadGateway = "Bad Gateway";
stock const ServiceUnavailable = "Service Unavailable";
stock const GatewayTimeout = "Gateway Timeout";
stock const HttpvNotSupported = "HTTP Version Not Supported";
stock const NegotiationLoop = "Variant Also Negotiates";
stock const NoStorage = "Insufficient Storage";
stock const LoopDetected = "Loop Detected";
stock const NotExtended = "Not Extended";
stock const AuthRequired = "Network Authentication Required";
