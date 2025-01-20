#pragma once

namespace vispro {

	class FeatureFlow {
	public:
		static void Compute(const char* velocityPathPrev, const char* velocityPathCurr, const char* velocityPathNext, int deltaSteps, const char* featureFlowPath);
	};

} // namespace vispro
