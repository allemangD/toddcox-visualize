#pragma once

#ifndef NDEBUG

void GLAPIENTRY log_gl_debug_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam
) {
    std::string s_source;
    switch(type){
        case GL_DEBUG_SOURCE_API:
            s_source = "API:";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            s_source = "WINDOW:";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            s_source = "SHADER:";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            s_source = "3P:";
        case GL_DEBUG_SOURCE_APPLICATION:
            s_source = "APP:";
        default:
            s_source = "";
    }

    std::string s_type;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        s_type = "ERROR:";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        s_type = "DEPRECATED:";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        s_type = "UNDEFINED:";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        s_type = "PORTABILITY:";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        s_type = "PERFORMANCE:";
        break;
    case GL_DEBUG_TYPE_MARKER:
        s_type = "MARKER:";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        s_type = "PUSH_GROUP:";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        s_type = "POP_GROUP:";
        break;
    default:
        s_type = "";
        break;
    }

    std::string s_severity;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        s_severity = "HIGH:";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        s_severity = "MED:";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        s_severity = "LOW:";
        break;
    default:
        s_severity = "INFO:";
        break;
    }

    std::cerr << "GL:" << s_source << s_type << s_severity << " " << message << std::endl;
}

#endif
