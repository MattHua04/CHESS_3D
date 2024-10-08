#include "graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

GLuint shaderProgram;
GLuint frameShaderProgram;

bool getSimilarVertexIndex_fast( 
	PackedVertex & packed, 
	std::map<PackedVertex,unsigned short> & VertexToOutIndex,
	unsigned short & result
){
	std::map<PackedVertex,unsigned short>::iterator it = VertexToOutIndex.find(packed);
	if ( it == VertexToOutIndex.end() ){
		return false;
	}else{
		result = it->second;
		return true;
	}
}

bool loadAssImp(
	const char * path, 
	vector<unsigned short> & indices,
	vector<glm::vec3> & vertices,
	vector<glm::vec2> & uvs,
	vector<glm::vec3> & normals
) {

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if( !scene) {
		fprintf( stderr, "%s", importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

	// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	// The "scene" pointer will be deleted automatically by "importer"
	return true;
}

void indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,
	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
){
	std::map<PackedVertex,unsigned short> VertexToOutIndex;

	// For each input vertex
	for ( unsigned int i=0; i<in_vertices.size(); i++ ){

		PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};
		

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex_fast( packed, VertexToOutIndex, index);

		if ( found ){ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back( index );
		}else{ // If not, it needs to be added in the output data.
			out_vertices.push_back( in_vertices[i]);
			out_uvs     .push_back( in_uvs[i]);
			out_normals .push_back( in_normals[i]);
			unsigned short newindex = (unsigned short)out_vertices.size() - 1;
			out_indices .push_back( newindex );
			VertexToOutIndex[ packed ] = newindex;
		}
	}
}

const char* VertexSourcePointer = R"(
   #version 330 core

    // Input vertex data, different for all executions of this shader.
    layout(location = 0) in vec3 vertexPosition_modelspace;
    layout(location = 1) in vec2 vertexUV;
    layout(location = 2) in vec3 vertexNormal; // Added normal input

    // Output data; will be interpolated for each fragment.
    out vec2 UV;
    out vec3 FragPos; // Pass fragment position to fragment shader
    out vec3 Normal; // Pass normal to fragment shader

    // Values that stay constant for the whole mesh.
    uniform mat4 MVP;
    uniform mat4 Model; // Added to get the correct fragment position

    void main() {
        // Output position of the vertex, in clip space: MVP * position
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

        // UV of the vertex. No special space for this one.
        UV = vertexUV;

        // Pass fragment position and normal to the fragment shader
        FragPos = vec3(Model * vec4(vertexPosition_modelspace, 1.0));
        Normal = normalize(mat3(transpose(inverse(Model))) * vertexNormal); // Transform normal to world space
    }
)";

const char* FragmentSourcePointer = R"(
	#version 330 core

	// Interpolated values from the vertex shaders
	in vec2 UV;
	in vec3 FragPos; // Received fragment position
	in vec3 Normal; // Received normal

	// Output data
	out vec3 color;

	// Values that stay constant for the whole mesh.
	uniform sampler2D textureSampler;
	uniform vec3 viewPos; // Camera/viewer position
	uniform vec3 defaultLightPos = vec3(0.0, -1.0, 0.0); // Default light position

	// Brightness factor to adjust the light intensity
	uniform float viewLightBrightness = 0.75f; // New uniform to control brightness
	uniform float defaultLightBrightness = 0.5f; // Default brightness factor

	void main() {
		// Output color = color of the texture at the specified UV
		vec3 textureColor = texture(textureSampler, UV).rgb;

		// Normalize the normal and calculate the light directions
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

		// Combine the results
		vec3 ambient = textureColor * viewLightBrightness * 0.5; // Increase ambient light
		vec3 diffuse = (diffView + diffDefault) * textureColor * vec3(1.0, 1.0, 1.0); // Diffuse light with white color
		vec3 specular = (specView + specDefault) * vec3(1.0, 1.0, 1.0); // Specular light with white color

		// Final color calculation
		color = ambient + diffuse + specular;
	}
)";

const char* FrameVertexSourcePointer = R"(
    #version 330 core
    layout (location = 0) in vec2 inPosition;
    layout (location = 1) in vec2 inTexCoords;

    out vec2 texCoords;

    void main()
    {
        texCoords = inTexCoords;
        gl_Position = vec4(inPosition, 0.0, 1.0);
    }
)";

const char* FrameFragmentSourcePointer = R"(
	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;
	uniform sampler2D screenTexture;
	uniform sampler2D overlayTexture;
	uniform float aspectRatio;

	uniform bool useOverlayTexture;
	uniform bool whiteInCheck;
    uniform bool whiteMated;
    uniform bool blackMated;
	uniform bool gameRunning;

	void main()
	{
		vec4 color = texture(screenTexture, texCoords);

		vec2 centeredCoords = texCoords - vec2(0.5, 0.5);
		centeredCoords.x *= aspectRatio; // Adjust x coordinate by the aspect ratio
		float dist = length(centeredCoords);

		if (gameRunning && dist < 0.01) {
			color.rgb = vec3(1.0) - color.rgb; // Invert the color
		}

		if (whiteMated) {
            color *= vec4(1.0f, 0.25f, 0.25f, 1.0f); // Darken color for white mated
        } else if (blackMated) {
            color *= vec4(0.25f, 1.0f, 0.25f, 1.0f); // Darken color for black mated
        } else if (whiteInCheck) {
            color *= vec4(1.0f, 0.5f, 0.5f, 1.0f); // Slightly darken color for white in check
        }

		// Apply overlay texture
		vec3 overlayColor = texture(overlayTexture, texCoords).rgb;
		if (useOverlayTexture && length(overlayColor - vec3(1.0, 0.0, 0.0)) > 0.7) {
			color.rgb = overlayColor;
		}

		FragColor = color;
	}
)";

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

GLuint loadDDS(const char * imagepath){

	unsigned char header[124];

	FILE *fp; 
 
	/* try to open the file */ 
	fp = fopen(imagepath, "rb"); 
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); 
		return 0;
	}
   
	/* verify the type of file */ 
	char filecode[4]; 
	fread(filecode, 1, 4, fp); 
	if (strncmp(filecode, "DDS ", 4) != 0) { 
		fclose(fp); 
		return 0; 
	}
	
	/* get the surface desc */ 
	fread(&header, 124, 1, fp); 

	unsigned int height      = *(unsigned int*)&(header[8 ]);
	unsigned int width	     = *(unsigned int*)&(header[12]);
	unsigned int linearSize	 = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC      = *(unsigned int*)&(header[80]);

 
	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */ 
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize; 
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char)); 
	fread(buffer, 1, bufsize, fp); 
	/* close the file pointer */ 
	fclose(fp);

	unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4; 
	unsigned int format;
	switch(fourCC) 
	{ 
	case FOURCC_DXT1: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
		break; 
	case FOURCC_DXT3: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
		break; 
	case FOURCC_DXT5: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
		break; 
	default: 
		free(buffer); 
		return 0; 
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);	
	
	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; 
	unsigned int offset = 0;

	/* load the mipmaps */ 
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) 
	{ 
		unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize; 
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,  
			0, size, buffer + offset); 
	 
		offset += size; 
		width  /= 2; 
		height /= 2; 

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if(width < 1) width = 1;
		if(height < 1) height = 1;

	} 

	free(buffer); 

	return textureID;
}

void shadersInit() {
    shaderProgram = loadShader(VertexSourcePointer, FragmentSourcePointer);
    frameShaderProgram = loadShader(FrameVertexSourcePointer, FrameFragmentSourcePointer);
}