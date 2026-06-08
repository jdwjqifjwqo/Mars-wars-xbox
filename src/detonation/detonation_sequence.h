#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace Mars3026 {

// ============================================================================
// Coordinate System for Mars Tactical Display
// ============================================================================
struct Coordinate {
    double latitude;      // Mars latitude
    double longitude;     // Mars longitude
    float altitude;       // Altitude in meters
    
    Coordinate(double lat = 0.0, double lon = 0.0, float alt = 0.0f)
        : latitude(lat), longitude(lon), altitude(alt) {}
    
    // ISO grid position conversion
    int GetISOGridX() const;
    int GetISOGridY() const;
};

// ============================================================================
// Detonation Event Timeline
// ============================================================================
enum class DetonationPhase {
    ARMED,                 // 00:00:02 ARMED
    COUNTDOWN,            // 00:00:00 DETONATION (countdown phase)
    ARMED_SECTOR,         // 00:06:50 SECTOR BREACHED
    EXTRACT_OVERWATCH,    // 00:24:56 EXTRACT OVERWATCH
    CAUTION_LOCKED,       // 00:25:23 CAUTION LOCKED
    DETONATION_COMPLETE,  // Sequence finished
};

struct DetonationEvent {
    DetonationPhase phase;
    std::chrono::milliseconds timestamp;
    Coordinate target_coordinate;
    std::string status_text;
    int iso_grid_section;
    
    DetonationEvent(
        DetonationPhase p,
        std::chrono::milliseconds ts,
        const Coordinate& coord,
        const std::string& status,
        int iso_section
    ) : phase(p), timestamp(ts), target_coordinate(coord),
        status_text(status), iso_grid_section(iso_section) {}
};

// ============================================================================
// Mars Detonation Sequence Controller (3026 Scenario)
// ============================================================================
class DetonationSequence {
public:
    DetonationSequence();
    ~DetonationSequence();
    
    // Initialize the 3026 detonation sequence
    void Initialize(const std::string& location = "cologny");
    
    // Start countdown from current phase
    void Start();
    void Pause();
    void Resume();
    void Stop();
    
    // Get current state
    DetonationPhase GetCurrentPhase() const { return current_phase_; }
    std::chrono::milliseconds GetElapsedTime() const;
    float GetCountdownProgress() const;  // 0.0 to 1.0
    
    // Coordinate tracking
    const Coordinate& GetTargetCoordinate() const { return current_coordinate_; }
    const std::vector<Coordinate>& GetTriangulatedCoordinates() const;
    
    // Eye-active vision triangulation
    void UpdateEyeTrackingData(float eye_x, float eye_y, float eye_z);
    void ComputeTriangulation();
    
    // ISO grid integration
    int GetCurrentISOGridSection() const { return current_iso_section_; }
    const std::string& GetCurrentISOGridID() const { return current_iso_grid_id_; }
    
    // UI Status strings
    const std::string& GetCountdownString() const { return countdown_string_; }
    const std::string& GetStatusText() const { return status_text_; }
    const std::string& GetCoordinateString() const { return coordinate_string_; }
    
    // Mars time tracking (Mars Sol & LST)
    const std::string& GetMarsTime() const { return mars_time_string_; }
    
    // Event callbacks
    using EventCallback = std::function<void(const DetonationEvent&)>;
    void RegisterPhaseCallback(EventCallback callback);
    
    // Render target data for Xbox platform conversion
    struct RenderTargetData {
        void* texture_data;
        int width;
        int height;
        int format;  // DXGI_FORMAT
    };
    
    RenderTargetData GetCurrentRenderTarget() const;
    
private:
    // Internal state
    DetonationPhase current_phase_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point pause_time_;
    bool is_paused_;
    bool is_running_;
    
    // Coordinate tracking
    Coordinate current_coordinate_;
    std::vector<Coordinate> triangulated_coordinates_;
    
    // ISO grid state
    int current_iso_section_;
    std::string current_iso_grid_id_;
    
    // Display strings
    std::string countdown_string_;
    std::string status_text_;
    std::string coordinate_string_;
    std::string mars_time_string_;
    
    // Event callbacks
    std::vector<EventCallback> phase_callbacks_;
    
    // Phase definition data
    std::vector<DetonationEvent> timeline_events_;
    
    // Helper functions
    void UpdateCountdownString();
    void UpdateMarsTime();
    void UpdateISOGridSection();
    void TriggerPhaseCallbacks(const DetonationEvent& event);
};

}  // namespace Mars3026
