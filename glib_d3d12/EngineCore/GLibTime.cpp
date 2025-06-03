#include <GLibTime.h>
#include <algorithm>
#include <deque>

glib::GLibTime* glib::GLibTime::instance = nullptr;

bool glib::GLibTime::m_Initialized = false;
std::chrono::high_resolution_clock::time_point glib::GLibTime::m_StartTime;
std::chrono::high_resolution_clock::time_point glib::GLibTime::m_LastTime;
std::chrono::high_resolution_clock::time_point glib::GLibTime::m_LevelLoadTime;

float glib::GLibTime::m_DeltaTime = 0.0f;
float glib::GLibTime::m_UnscaledDeltaTime = 0.0f;
float glib::GLibTime::m_SmoothDeltaTime = 0.0f;
float glib::GLibTime::m_FixedDeltaTime = 1.0f / 60.0f;
float glib::GLibTime::m_TimeScale = 1.0f;
float glib::GLibTime::m_MaximumDeltaTime = 0.3333f;
int glib::GLibTime::m_FrameCount = 0;
int glib::GLibTime::m_HitStop = 0;
bool glib::GLibTime::m_InFixedTimeStep = false;

static std::deque<float> deltaHistory;

void glib::GLibTime::initialize()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
    m_LastTime = m_StartTime;
    m_LevelLoadTime = m_StartTime;
    m_Initialized = true;
    m_FrameCount = 0;
    m_SmoothDeltaTime = 0.0f;
    m_HitStop = 0;
}

void glib::GLibTime::Update()
{
    if (!m_Initialized)
        initialize();

    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = currentTime - m_LastTime;

    // deltaTime制限（最大値補正）
    m_UnscaledDeltaTime = std::min(delta.count(), m_MaximumDeltaTime);
    m_DeltaTime = m_UnscaledDeltaTime * m_TimeScale;
    m_LastTime = currentTime;
    m_FrameCount++;

    // delta履歴から平均を出す（SmoothDeltaTime用）
    const size_t windowSize = 10;
    deltaHistory.push_back(m_DeltaTime);
    if (deltaHistory.size() > windowSize)
        deltaHistory.pop_front();

    float sum = 0.0f;
    for (float dt : deltaHistory) sum += dt;
    m_SmoothDeltaTime = sum / deltaHistory.size();

    m_HitStop = std::max(m_HitStop - 1, 0);
}

void glib::GLibTime::BeginFixedUpdate()
{
    m_InFixedTimeStep = true;
}

void glib::GLibTime::EndFixedUpdate()
{
    m_InFixedTimeStep = false;
}

void glib::GLibTime::SetLevelLoaded()
{
    if (!m_Initialized)
        initialize();
    m_LevelLoadTime = std::chrono::high_resolution_clock::now();
}

float glib::GLibTime::DeltaTime() 
{
    return m_DeltaTime; 
}
float glib::GLibTime::UnscaledDeltaTime() { return m_UnscaledDeltaTime; }
float glib::GLibTime::FixedDeltaTime() { return m_FixedDeltaTime; }
float glib::GLibTime::FixedUnscaledDeltaTime() { return m_FixedDeltaTime; }
float glib::GLibTime::TotalTime() { return UnscaledTime() * m_TimeScale; }
int glib::GLibTime::TotalTimeInt() { return static_cast<int>(TotalTime()); }

float glib::GLibTime::UnscaledTime()
{
    if (!m_Initialized) initialize();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - m_StartTime;
    return elapsed.count();
}

float glib::GLibTime::RealtimeSinceStartup() { return UnscaledTime(); }

float glib::GLibTime::TimeSinceLevelLoad()
{
    if (!m_Initialized) initialize();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - m_LevelLoadTime;
    return elapsed.count() * m_TimeScale;
}

int glib::GLibTime::FrameCount() { return m_FrameCount; }
bool glib::GLibTime::InFixedTimeStep() { return m_InFixedTimeStep; }
float glib::GLibTime::SmoothDeltaTime() { return m_SmoothDeltaTime; }
float glib::GLibTime::MaximumDeltaTime() { return m_MaximumDeltaTime; }

void glib::GLibTime::SetTimeScale(float scale) { m_TimeScale = scale; }
float glib::GLibTime::GetTimeScale() { return m_TimeScale; }
void glib::GLibTime::SetFixedDeltaTime(float fixed) { m_FixedDeltaTime = fixed; }
void glib::GLibTime::SetMaximumDeltaTime(float max) { m_MaximumDeltaTime = max; }

void glib::GLibTime::SetHitStop(int frame)
{
    if (m_HitStop < frame)
        m_HitStop = frame;
}

int glib::GLibTime::HitStop()
{
    return m_HitStop;
}

bool glib::GLibTime::IsHitStop()
{
    return (m_HitStop > 0);
}
