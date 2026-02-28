#include "Renderer.h"

#include "Application.h"
#include "Application.h"


void Renderer::SetShader(const Ref<Shader>& shader)
{
    m_ActiveShader = shader;
    if (m_ActiveShader)
        m_ActiveShader->Bind();
}

void Renderer::SetMVP(const glm::mat4& mvp)
{
    assert(m_ActiveShader);
    m_ActiveShader->SetMat4("u_MVP", mvp);
}

void Renderer::BindVertexArray(GLuint vao) const
{
    glBindVertexArray(vao);
}

void Renderer::BindCubemap(GLuint textureID, uint32_t slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}


void Renderer::Clear() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Renderer::SetClearColor(const glm::vec4& color) const
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::EnableDepthTest(bool enabled) const
{
    enabled ? glEnable(GL_DEPTH_TEST)
            : glDisable(GL_DEPTH_TEST);
}

void Renderer::EnableDepthMask(bool enabled) const
{
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void Renderer::SetDepthFunc(unsigned int func) const
{
    glDepthFunc(func);
}

void Renderer::EnableStencilTest(bool enabled) const
{
    enabled ? glEnable(GL_STENCIL_TEST)
            : glDisable(GL_STENCIL_TEST);
}

void Renderer::EnableBlending(bool enabled) const
{
    enabled ? glEnable(GL_BLEND)
            : glDisable(GL_BLEND);
}

void Renderer::SetBlendFunc(unsigned int src, unsigned int dst) const
{
    glBlendFunc(src, dst);
}

void Renderer::EnableCulling(bool enabled) const
{
    enabled ? glEnable(GL_CULL_FACE)
            : glDisable(GL_CULL_FACE);
}

void Renderer::SetPolygonMode(unsigned int face, unsigned int mode) const
{
    glPolygonMode(face, mode);
}

void Renderer::EnablePolygonOffset(bool enabled) const
{
    enabled ? glEnable(GL_POLYGON_OFFSET_LINE)
            : glDisable(GL_POLYGON_OFFSET_LINE);
}

void Renderer::SetPolygonOffset(float factor, float units) const
{
    glPolygonOffset(factor, units);
}

void Renderer::SetViewport(int x, int y, int width, int height) const
{
    glViewport(x, y, width, height);
}



void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib) const
{
    va.Bind();
    ib.Bind();
    glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawLines(const VertexArray& va, const IndexBuffer& ib) const
{
    va.Bind();
    ib.Bind();
    glDrawElements(GL_LINES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawArrays(unsigned int mode,
                          unsigned int first,
                          unsigned int count) const
{
    glDrawArrays(mode, first, count);
}

void Renderer::DrawOutline(const glm::mat4& mvp,
                           const VertexArray& va,
                           const IndexBuffer& ib)
{

}


void Renderer::DrawWireframe(const Ref<Shader>& outlineShader, const glm::vec3& color,
                             const glm::mat4& MVP, const VertexArray& va, const IndexBuffer& ib)
{
	glEnable(GL_POLYGON_OFFSET_LINE);

	glPolygonOffset(-1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	// glLineWidth(2.0f);
	SetShader(outlineShader);
	outlineShader->Bind();
    outlineShader->SetVec4("u_OutlineColor", glm::vec4(color, 1.0f));
	outlineShader->SetMat4("u_MVP", MVP);
	Draw(va, ib);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_POLYGON_OFFSET_LINE);
}

void Renderer::InitOutlines(int width, int height)
{
    m_SelectionWidth  = width;
    m_SelectionHeight = height;

    glGenFramebuffers(1, &m_SelectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SelectionFBO);

    glGenTextures(1, &m_SelectionTex);
    glBindTexture(GL_TEXTURE_2D, m_SelectionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                 width, height,
                 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           m_SelectionTex,
                           0);

    glGenRenderbuffers(1, &m_SelectionDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_SelectionDepth);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              m_SelectionDepth);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &m_FullscreenVAO);
}



void Renderer::ResizeOutlines(int width, int height)
{
    m_SelectionWidth  = width;
    m_SelectionHeight = height;

    glBindTexture(GL_TEXTURE_2D, m_SelectionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                 width, height,
                 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, m_SelectionDepth);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          width, height);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


void Renderer::BeginFrame(int frameWidth, int frameHeight, int vpX, int vpY, int vpW, int vpH)
{
    m_FrameWidth = frameWidth;
    m_FrameHeight = frameHeight;
    m_VpX = vpX; m_VpY = vpY; m_VpW = vpW; m_VpH = vpH;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(vpX, vpY, vpW, vpH);


    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    GLint fb = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}




void Renderer::BeginSelectionMask()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_SelectionFBO);
    glViewport(0, 0, m_SelectionWidth, m_SelectionHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    // TODO: Fix later
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


void Renderer::EndSelectionMask()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_FrameWidth, m_FrameHeight);
}



void Renderer::DrawSelectionMask(const Ref<Shader>& maskShader,
                                 const glm::mat4& mvp,
                                 const VertexArray& va,
                                 const IndexBuffer& ib)
{
    maskShader->Bind();
    maskShader->SetMat4("u_MVP", mvp);

    va.Bind();
    ib.Bind();
    glDrawElements(GL_TRIANGLES,
                   ib.GetCount(),
                   GL_UNSIGNED_INT,
                   nullptr);
}

void Renderer::CompositeOutlines(const Ref<Shader>& outlineShader,
                                 const glm::vec3& outlineColor,
                                 int thicknessPixels)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(m_VpX, m_VpY, m_VpW, m_VpH);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    outlineShader->Bind();
    outlineShader->SetVec3("u_OutlineColor", outlineColor);
    outlineShader->SetInt("u_ThicknessPx", thicknessPixels);
    outlineShader->SetVec2("u_TexelSize",
        glm::vec2(1.0f / m_SelectionWidth,
                  1.0f / m_SelectionHeight));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_SelectionTex);
    outlineShader->SetInt("u_MaskTex", 0);

    glBindVertexArray(m_FullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}


