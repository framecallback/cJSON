#!/bin/bash

SCRIPT_DIR=`dirname $0`
TESTS_DIR=$SCRIPT_DIR/build/tests

function run_test() {
	echo "======================================================"
	echo "test: $1"
	echo "======================================================"
	$1 || exit -1
}

cd $TESTS_DIR
run_test ./../cJSON_test
run_test ./cjson_add
run_test ./compare_tests
run_test ./minify_tests
run_test ./misc_tests
run_test ./parse_array
run_test ./parse_examples
run_test ./parse_hex4
run_test ./parse_number
run_test ./parse_object
run_test ./parse_string
run_test ./parse_value
run_test ./parse_with_opts
run_test ./print_array
run_test ./print_number
run_test ./print_object
run_test ./print_string
run_test ./print_value
run_test ./readme_examples

echo "======================================================"
echo "    OK, all tests passed!"
echo "======================================================"