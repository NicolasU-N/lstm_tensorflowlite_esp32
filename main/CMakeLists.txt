# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.
# output_handler.cc 
idf_component_register(
    SRCS main.cc main_functions.cc constants.cc output_handler.cc model_data.cc # list the source files of this component
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES esp_timer         # optional, list the public requirements (component names)
    PRIV_REQUIRES spi_flash       # optional, list the private requirements
)