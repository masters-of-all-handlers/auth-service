import pytest

# Start the tests via `make test-debug` or `make test-release`


@pytest.mark.pgsql('db-1', files=['initial_data.sql'])
@pytest.mark.parametrize(
    'login, password', [
        ('me@ya.ru', 'pswd'),
        ('email@email.com', 'qwerty'),
    ]
)
async def test_login_entry_before_reg(service_client, login, password):
    data = {
        'login': login,
        'password': password,
    }

    default_user = {
        'login': 'admin',
        'password': 'yandexthebest',
    }

    head_json = {
        'Content-Type': 'application/json'
    }

    # data = {'email': email, 'password': password}

    response = await service_client.post(
        '/login', json=data, headers=head_json
    )
    assert response.status_code == 404

    response = await service_client.post(
        '/register', json=data, headers=head_json
    )
    assert response.status_code == 401

    response = await service_client.post(
        '/login', json=default_user, headers=head_json
    )
    assert response.status_code == 200
    head = {
        'X-Ya-User-Ticket': response.json()['id'],
        'Content-Type': 'application/json'
    }
    response = await service_client.post(
        '/register', json=data, headers=head
    )
    assert response.status_code == 200

    response = await service_client.post(
        '/login', json=data, headers=head_json
    )
    head = {
        'X-Ya-User-Ticket': response.json()['id'],
        'Content-Type': 'application/json'
    }
    response = await service_client.post(
        '/check', headers=head
    )
    assert response.status_code == 200
