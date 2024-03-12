#pragma once

// Quote state
typedef enum {
	QUOTE_NONE,
	QUOTE_SINGLE,
	QUOTE_DOUBLE
} quote_st;

// Check if character is considered a blank
#define is_blank(x) (x == ' ' || x == '\t' || x == '\0')
