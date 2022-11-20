import pytest

# Start the tests via `make test-debug` or `make test-release`


async def test_login_entry_before_reg(service_client):
    response = await service_client.options("/something/new")
    assert response.headers["Access-Control-Allow-Origin"] == "*"
    assert response.status == 204
    assert response.text == ""
