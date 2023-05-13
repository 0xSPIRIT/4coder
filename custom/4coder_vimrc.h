
// EXAMPLE 4coder_vimrc.h

#define VIM_DO_ANIMATE                 1
#define VIM_USE_TRADITIONAL_CHORDS     1
#define VIM_USE_REIGSTER_BUFFER 1

enum EXAMPLE_Vim_Request{
	EXAMPLE_REQUEST_Title,
	EXAMPLE_REQUEST_Rot13,

	EXAMPLE_REQUEST_COUNT
};

enum EXAMPLE_Vim_Objects{
	EXAMPLE_OBJECT_camel,

	EXAMPLE_OBJECT_COUNT
};
#define VIM_ADDITIONAL_REQUESTS        EXAMPLE_REQUEST_COUNT
#define VIM_ADDITIONAL_TEXT_OBJECTS    EXAMPLE_OBJECT_COUNT
#define VIM_ADDITIONAL_PEEK            0

#define VIM_DEFAULT_REGISTER           unnamed

#define VIM_USE_BOTTOM_LISTER          0
#define VIM_LISTER_RANGE               3,5
#define VIM_LISTER_MAX_RATIO           0.25f
