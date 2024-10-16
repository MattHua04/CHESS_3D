#include "graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

GLuint shaderProgram;
GLuint frameShaderProgram;

const char* VertexSourcePointer = R"(
   #version 330 core

    // Input vertex data
    layout(location = 0) in vec3 vertexPosition_modelspace;
    layout(location = 1) in vec2 vertexUV;
    layout(location = 2) in vec3 vertexNormal;

    // Output data
    out vec2 UV;
    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 MVP;
    uniform mat4 Model;

    void main() {
        // Output position of the vertex in clip space
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

        // UV of the vertex
        UV = vertexUV;

        // FragmentShader inputs
        FragPos = vec3(Model * vec4(vertexPosition_modelspace, 1.0));
        Normal = normalize(mat3(transpose(inverse(Model))) * vertexNormal); // Transform normal to world space
    }
)";

const char* FragmentSourcePointer = R"(
	#version 330 core

	// Inputs from VertexShader
	in vec2 UV;
	in vec3 FragPos;
	in vec3 Normal;

	// Output data
	out vec3 color;

	uniform sampler2D textureSampler;
	uniform vec3 viewPos; // Camera/viewer position
	uniform vec3 defaultLightPos = vec3(0.0, -1.0, 0.0);

	// Light intensity
	uniform float viewLightBrightness = 0.75f;
	uniform float defaultLightBrightness = 0.5f;

	void main() {
		// Default texture color
		vec3 textureColor = texture(textureSampler, UV).rgb;

		// Normalize the normal and calculate light directions
		vec3 norm = normalize(Normal);
		vec3 lightDirView = normalize(viewPos - FragPos);
		vec3 lightDirDefault = normalize(defaultLightPos - FragPos);

		// Calculate the diffuse components
		float diffView = max(dot(norm, lightDirView), 0.0) * viewLightBrightness; // Scale by brightness
		float diffDefault = max(dot(norm, lightDirDefault), 0.0) * defaultLightBrightness; // Scale by brightness

		// Calculate the specular components
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDirView = reflect(-lightDirView, norm);
		vec3 reflectDirDefault = reflect(-lightDirDefault, norm);
		float specView = pow(max(dot(viewDir, reflectDirView), 0.0), 5.0) * viewLightBrightness; // Reduce specular intensity
		float specDefault = pow(max(dot(viewDir, reflectDirDefault), 0.0), 5.0) * defaultLightBrightness; // Reduce specular intensity

		vec3 ambient = textureColor * viewLightBrightness * 0.5; // Increase ambient light
		vec3 diffuse = (diffView + diffDefault) * textureColor * vec3(1.0, 1.0, 1.0); // Diffuse light with white color
		vec3 specular = (specView + specDefault) * vec3(1.0, 1.0, 1.0); // Specular light with white color

		color = ambient + diffuse + specular;
	}
)";

const char* FrameVertexSourcePointer = R"(
    #version 330 core

	// Input vertex data
    layout (location = 0) in vec2 inPosition;
    layout (location = 1) in vec2 inTexCoords;

	// Output data
    out vec2 texCoords;

    void main()
    {
        texCoords = inTexCoords;
        gl_Position = vec4(inPosition, 0.0, 1.0);
    }
)";

const char* FrameFragmentSourcePointer = R"(
	#version 330 core

	// Input data
	in vec2 texCoords;

	// Output data
	out vec4 FragColor;

	// Texture data
	uniform sampler2D screenTexture;
	uniform sampler2D overlayTexture;
	uniform float aspectRatio;

	uniform bool useOverlayTexture;
	uniform bool playerInCheck;
    uniform bool playerMated;
    uniform bool opponentMated;
	uniform bool gameRunning;

	void main()
	{
		vec4 color = texture(screenTexture, texCoords);

		vec2 centeredCoords = texCoords - vec2(0.5, 0.5);
		centeredCoords.x *= aspectRatio; // Account for aspect ratio
		float dist = length(centeredCoords);

		if (gameRunning && dist < 0.01) {
			color.rgb = vec3(1.0) - color.rgb; // Invert the color around crosshair
		}

		if (playerMated) {
            color *= vec4(1.0f, 0.25f, 0.25f, 1.0f); // Red shift
        } else if (opponentMated) {
            color *= vec4(0.25f, 1.0f, 0.25f, 1.0f); // Green shift
        } else if (playerInCheck) {
            color *= vec4(1.0f, 0.5f, 0.5f, 1.0f); // Slighly red shift
        }

		// Apply overlay texture if needed
		vec3 overlayColor = texture(overlayTexture, texCoords).rgb;
		if (useOverlayTexture && length(overlayColor - vec3(1.0, 0.0, 0.0)) > 0.7) {
			color.rgb = overlayColor;
		}

		FragColor = color;
	}
)";

void shadersInit() {
    shaderProgram = loadShader(VertexSourcePointer, FragmentSourcePointer);
    frameShaderProgram = loadShader(FrameVertexSourcePointer, FrameFragmentSourcePointer);
}

bool loadAssImp(
	const char * path, 
	vector<unsigned short> & indices,
	vector<glm::vec3> & vertices,
	vector<glm::vec2> & uvs,
	vector<glm::vec3> & normals
) {

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0);
	if (!scene) {
		fprintf( stderr, "%s", importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0];

	// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for (unsigned int i=0; i<mesh->mNumVertices; i++) {
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for (unsigned int i=0; i<mesh->mNumVertices; i++) {
		aiVector3D UVW = mesh->mTextureCoords[0][i];
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for (unsigned int i=0; i<mesh->mNumVertices; i++) {
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}

	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++) {
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	return true;
}

GLuint loadShader(const char* VertexSourcePointer, const char* FragmentSourcePointer) {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    GLuint ShaderID = glCreateProgram();
    glAttachShader(ShaderID, VertexShaderID);
    glAttachShader(ShaderID, FragmentShaderID);
    glLinkProgram(ShaderID);
    glGetProgramiv(ShaderID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ShaderID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ShaderID, VertexShaderID);
    glDetachShader(ShaderID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ShaderID;
}

GLuint loadTexture(const char * imagepath) {
	stbi_set_flip_vertically_on_load(true);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, channels;
    unsigned char* image = stbi_load(imagepath, &width, &height, &channels, 4);
    if (image == NULL) {
        std::cerr << "Failed to load texture: " << imagepath << std::endl;
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    
    return textureID;
}