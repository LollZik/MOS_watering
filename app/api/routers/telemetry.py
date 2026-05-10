from fastapi import APIRouter, Depends, status
from schemas.telemetry import TelemetryCreate, TelemetryStatusResponse
from services.telemetry_service import TelemetryService
from api.dependencies import get_telemetry_service, verify_api_key

router = APIRouter(prefix="/telemetry", tags=["Telemetry"])

@router.post("", response_model=TelemetryStatusResponse, status_code=status.HTTP_201_CREATED)
async def post_telemetry(
    data: TelemetryCreate,
    telemetry_service: TelemetryService = Depends(get_telemetry_service),
    api_key: str = Depends(verify_api_key)
):
    return await telemetry_service.save_telemetry(data)
