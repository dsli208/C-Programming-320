#include <criterion/criterion.h>
#include "url.h"
#include "http.h"

Test(url_suite, parse_normal_url) {
    URL *url;
    char *str = "http://www.google.com";
    url = url_parse(str);
    cr_assert_neq(url, NULL);
    cr_assert_eq(url_port(url), 80);
    cr_assert_str_eq(url_method(url), "http");
    cr_assert_str_eq(url_hostname(url), "www.google.com");
    cr_assert_str_eq(url_path(url), "/");
}

Test(url_suite, url_no_slash) {
    URL *url;
    url = url_parse("http:www.google.com");
    cr_assert_neq(url, NULL);
}

Test(url_suite, url_no_method) {
    URL *url;
    url = url_parse("//www.google.com");
    cr_assert_neq(url, NULL);
    cr_assert_eq(url_port(url), 0);
    cr_assert_eq(url_method(url), NULL);
    cr_assert_eq(url_hostname(url), NULL);
    cr_assert_str_eq(url_path(url), "//www.google.com");
}

Test(http_suite, basic_test) {
    URL *url = url_parse("http://www.google.com");
    cr_assert_neq(url, NULL);
    HTTP *http = http_open(url_address(url), url_port(url));
    cr_assert_neq(http, NULL);
    http_request(http, url);
    http_response(http);
    int code;
    char *status = http_status(http, &code);
    cr_assert_str_eq(status, "HTTP/1.0 200 OK");
    cr_assert_eq(code, 200);
    http_close(http);
}

Test(http_suite, query_header_test) {
    URL *url = url_parse("http://bsd7.cs.stonybrook.edu/index.html");
    cr_assert_neq(url, NULL);
    HTTP *http = http_open(url_address(url), url_port(url));
    cr_assert_neq(http, NULL);
    http_request(http, url);
    http_response(http);
    int code;
    char *status = http_status(http, &code);
    cr_assert_str_eq(status, "HTTP/1.1 200 OK");
    cr_assert_eq(code, 200);
    cr_assert_str_eq(http_headers_lookup(http, "content-length"), "11510");
    cr_assert_str_eq(http_headers_lookup(http, "content-type"), "text/html");
    http_close(http);
}

Test(http_suite, not_found_test) {
    URL *url = url_parse("http://www.google.com/no_such_document.html");
    cr_assert_neq(url, NULL);
    HTTP *http = http_open(url_address(url), url_port(url));
    cr_assert_neq(http, NULL);
    http_request(http, url);
    http_response(http);
    int code;
    char *status = http_status(http, &code);
    cr_assert_str_eq(status, "HTTP/1.0 404 Not Found");
    cr_assert_eq(code, 404);
    http_close(http);
}

//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################

