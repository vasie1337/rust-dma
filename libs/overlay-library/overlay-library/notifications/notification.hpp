#pragma once
#include "../include.hpp"

enum class NotificationPosition {
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class NotificationType {
    Info,
    Warning,
    Error,
    Debug,
    Success
};

class Notification {
private:
    ImVec4 GetBackgroundColor() const {
        const float alpha = 0.95f;
        switch (Type) {
        case NotificationType::Info:
            return ImVec4(0.12f, 0.14f, 0.18f, alpha);
        case NotificationType::Warning:
            return ImVec4(0.18f, 0.16f, 0.12f, alpha);
        case NotificationType::Error:
            return ImVec4(0.25f, 0.12f, 0.12f, alpha);
        case NotificationType::Debug:
            return ImVec4(0.12f, 0.12f, 0.18f, alpha);
        case NotificationType::Success:
            return ImVec4(0.12f, 0.18f, 0.12f, alpha);
        }
        return ImVec4(0.12f, 0.12f, 0.12f, alpha);
    }

    ImVec4 GetTitleColor() const {
        switch (Type) {
        case NotificationType::Info:
            return ImVec4(0.8f, 0.9f, 1.0f, Alpha);
        case NotificationType::Warning:
            return ImVec4(1.0f, 0.9f, 0.8f, Alpha);
        case NotificationType::Error:
            return ImVec4(1.0f, 0.8f, 0.8f, Alpha);
        case NotificationType::Debug:
            return ImVec4(0.9f, 0.8f, 1.0f, Alpha);
        case NotificationType::Success:
            return ImVec4(0.8f, 1.0f, 0.8f, Alpha);
        }
        return ImVec4(1.0f, 1.0f, 1.0f, Alpha);
    }

    ImVec4 GetProgressBarColor() const {
        switch (Type) {
        case NotificationType::Info:
            return ImVec4(0.4f, 0.7f, 1.0f, Alpha);
        case NotificationType::Warning:
            return ImVec4(1.0f, 0.7f, 0.2f, Alpha);
        case NotificationType::Error:
            return ImVec4(1.0f, 0.4f, 0.4f, Alpha);
        case NotificationType::Debug:
            return ImVec4(0.6f, 0.5f, 0.8f, Alpha);
        case NotificationType::Success:
            return ImVec4(0.4f, 0.8f, 0.4f, Alpha);
        }
        return ImVec4(0.4f, 0.8f, 1.0f, Alpha);
    }

    ImVec4 GetMessageColor() const {
        switch (Type) {
        case NotificationType::Info:
            return ImVec4(0.9f, 0.95f, 1.0f, Alpha * 0.8f);
        case NotificationType::Warning:
            return ImVec4(1.0f, 0.95f, 0.9f, Alpha * 0.8f);
        case NotificationType::Error:
            return ImVec4(1.0f, 0.9f, 0.9f, Alpha * 0.8f);
        case NotificationType::Debug:
            return ImVec4(0.95f, 0.9f, 1.0f, Alpha * 0.8f);
        case NotificationType::Success:
            return ImVec4(0.9f, 1.0f, 0.9f, Alpha * 0.8f);
        }
        return ImVec4(0.9f, 0.9f, 0.9f, Alpha * 0.8f);
    }

    const char* GetIcon() const {
        switch (Type) {
        case NotificationType::Info:
            return ICON_FA_INFO_CIRCLE;
        case NotificationType::Warning:
            return ICON_FA_EXCLAMATION_TRIANGLE;
        case NotificationType::Error:
            return ICON_FA_TIMES_CIRCLE;
        case NotificationType::Debug:
            return ICON_FA_BUG;
        case NotificationType::Success:
            return ICON_FA_CHECK_CIRCLE;
        }
        return ICON_FA_BELL;
    }

public:
    Notification(const std::string& title, const std::string& message, NotificationType type, float duration, bool persistent = false)
        : Title(title)
        , Message(message)
        , Type(type)
        , Duration(duration)
        , StartTime(static_cast<float>(ImGui::GetTime()))
        , EndTime(StartTime + Duration)
        , IsPersistent(persistent)
        , Alpha(0.0f)
        , Scale(0.8f)
        , IsAlive(true)
        , YOffset(0.0f)
        , TargetYOffset(0.0f)
    {
        TextSize = ImGui::CalcTextSize(Message.c_str());
        WindowSize.x = 360.0f;
        WindowSize.y = 72.0f;
        ContentPadding = ImVec2(20.0f, 16.0f);
        IconScale = 1.f;
    }

    bool Render(const ImVec2& basePosition, float targetOffset)
    {
        if (!IsAlive) return false;

        float currentTime = static_cast<float>(ImGui::GetTime());
        float deltaTime = ImGui::GetIO().DeltaTime;

        const float fadeTime = 0.15f;
        const float moveSpeed = 12.0f;
        const float scaleSpeed = 8.0f;

        TargetYOffset = targetOffset;
        YOffset = ImLerp(YOffset, TargetYOffset, deltaTime * moveSpeed);

        if (!IsPersistent && currentTime >= EndTime) {
            Alpha = std::max<float>(0.0f, Alpha - (deltaTime / fadeTime));
            Scale = std::max<float>(0.8f, Scale - (deltaTime * scaleSpeed));
            if (Alpha <= 0.0f) {
                IsAlive = false;
                return false;
            }
        }
        else {
            Alpha = std::min<float>(1.0f, Alpha + (deltaTime / fadeTime));
            Scale = std::min<float>(1.0f, Scale + (deltaTime * scaleSpeed));
        }

        ImVec2 position = basePosition;
        position.y += YOffset;

        ImVec2 scaledSize = ImVec2(WindowSize.x * Scale, WindowSize.y * Scale);

        if (basePosition.x > ImGui::GetIO().DisplaySize.x * 0.5f) {
            position.x -= scaledSize.x;
        }
        else if (std::abs(basePosition.x - ImGui::GetIO().DisplaySize.x * 0.5f) < 1.0f) {
            position.x -= scaledSize.x * 0.5f;
        }

        ImGui::SetNextWindowPos(position);
        ImGui::SetNextWindowSize(scaledSize);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ContentPadding);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, GetBackgroundColor());

        char windowID[128];
        snprintf(windowID, sizeof(windowID), "%s##%p", Title.c_str(), static_cast<void*>(this));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin(windowID, nullptr, flags);

        ImGui::BeginGroup();

        ImGui::PushStyleColor(ImGuiCol_Text, GetTitleColor());

        float iconSize = ImGui::GetFontSize() * IconScale;
        ImGui::Text("%s", GetIcon());
        ImGui::SameLine(0, ContentPadding.x * 0.5f);

        ImGui::Text("%s", Title.c_str());
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GetMessageColor());
        ImGui::Text("%s", Message.c_str());
        ImGui::PopStyleColor();

        ImGui::EndGroup();

        if (!IsPersistent && currentTime < EndTime) {
            ImGui::SetCursorPosY(WindowSize.y - ContentPadding.y - 2.0f);
            float progress = 1.0f - ((currentTime - StartTime) / Duration);

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, GetProgressBarColor());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::ProgressBar(progress, ImVec2(-1, 2));
            ImGui::PopStyleColor(2);
        }

        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(4);

        return true;
    }

    bool IsExpired() const { return !IsAlive; }
    ImVec2 GetSize() const { return WindowSize; }

private:
    std::string Title;
    std::string Message;
    NotificationType Type;
    float Duration;
    float StartTime;
    float EndTime;
    bool IsPersistent;
    float Alpha;
    float Scale;
    bool IsAlive;
    ImVec2 TextSize;
    ImVec2 WindowSize;
    ImVec2 ContentPadding;
    float YOffset;
    float TargetYOffset;
	float IconScale;
};

class NotificationSystem {
public:
    NotificationSystem(NotificationPosition position = NotificationPosition::TopCenter)
        : Position(position)
        , MaxVisible(3)
        , StackSpacing(6.0f)
    {}

    void SetPosition(NotificationPosition position) {
        Position = position;
    }

    void AddNotification(const std::string& title, const std::string& message, NotificationType type, float duration = 4.0f, bool persistent = false) {
        Notifications.emplace_back(title, message, type, duration, persistent);
    }

    void ClearNotifications() {
        Notifications.clear();
    }

    void RenderNotifications() {
        if (Notifications.empty()) return;

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 basePos = GetBasePosition(screenSize);

        Notifications.erase(
            std::remove_if(Notifications.begin(), Notifications.end(),
                [](const Notification& notif) { return notif.IsExpired(); }),
            Notifications.end()
        );

        float currentOffset = 0.0f;
        size_t visibleCount = 0;

        for (auto& notification : Notifications) {
            if (visibleCount < MaxVisible) {
                float targetOffset = currentOffset;
                if (Position == NotificationPosition::BottomLeft ||
                    Position == NotificationPosition::BottomCenter ||
                    Position == NotificationPosition::BottomRight) {
                    targetOffset = -currentOffset;
                }

                if (notification.Render(basePos, targetOffset)) {
                    currentOffset += notification.GetSize().y + StackSpacing;
                    visibleCount++;
                }
            }
        }
    }

private:
    std::vector<Notification> Notifications;
    NotificationPosition Position;
    size_t MaxVisible;
    float StackSpacing;

    ImVec2 GetBasePosition(const ImVec2& screenSize) const {
        const float padding = 24.0f;
        ImVec2 basePos;

        switch (Position) {
        case NotificationPosition::TopLeft:
            basePos = ImVec2(padding, padding);
            break;
        case NotificationPosition::TopCenter:
            basePos = ImVec2(screenSize.x * 0.5f, padding);
            break;
        case NotificationPosition::TopRight:
            basePos = ImVec2(screenSize.x - padding, padding);
            break;
        case NotificationPosition::BottomLeft:
            basePos = ImVec2(padding, screenSize.y - padding);
            break;
        case NotificationPosition::BottomCenter:
            basePos = ImVec2(screenSize.x * 0.5f, screenSize.y - padding);
            break;
        case NotificationPosition::BottomRight:
            basePos = ImVec2(screenSize.x - padding, screenSize.y - padding);
            break;
        }

        return basePos;
    }
};