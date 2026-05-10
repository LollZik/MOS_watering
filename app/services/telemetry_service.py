import logging
from datetime import datetime, timezone
import asyncpg
from schemas.telemetry import TelemetryCreate

logger = logging.getLogger(__name__)

class TelemetryService:
    def __init__(self, db_session: asyncpg.Connection = None):
        self.db = db_session

    async def save_telemetry(self, telemetry: TelemetryCreate) -> dict:
        if not self.db:
            raise RuntimeError("Brak połączenia z bazą danych")
            
        current_time = datetime.now(timezone.utc)
        
        # 1. Zapis telemetrii do tabeli TimescaleDB (ignorujemy błędy FK w PoC dla testowania,
        # lub po prostu rzucamy wyjątkiem jeśli device_id nie istnieje)
        query = """
            INSERT INTO telemetry (time, device_id, water_lvl, battery_lvl, moisture_lvl, uptime)
            VALUES ($1, $2, $3, $4, $5, $6)
        """
        
        try:
            await self.db.execute(
                query,
                current_time,
                telemetry.device_id,
                telemetry.water_lvl,
                telemetry.battery_lvl,
                telemetry.moisture_lvl,
                telemetry.uptime
            )
            logger.info(f"Zapisano telemetrię z urządzenia {telemetry.device_id} do TimescaleDB.")
        except asyncpg.exceptions.ForeignKeyViolationError:
            # Urządzenie musi być zarejestrowane w tabeli `devices` zanim przyjmiemy telemetrię
            logger.warning(f"Odrzucono telemetrię: urządzenie {telemetry.device_id} nie istnieje w bazie.")
            raise ValueError(f"Urządzenie o ID {telemetry.device_id} nie jest zarejestrowane.")
        except Exception as e:
            logger.error(f"Błąd DB podczas zapisu telemetrii: {e}")
            raise e

        return {"status": "ok"}
