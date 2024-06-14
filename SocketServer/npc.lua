myid = 99999;

function set_uid(x)
   myid = x;
end

function event_player_mov1e(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
      end
   end
end

function event_player_move(player)
    player_x = API_get_x(player)
    player_y = API_get_y(player)
    my_x = API_get_x(myid)
    my_y = API_get_y(myid)

    -- �÷��̾��� ��ġ�� NPC ��ġ�� ���̸� ���Ͽ� ������ �����մϴ�.
    local dx = player_x - my_x
    local dy = player_y - my_y

    if dx > 0 then
        my_x = my_x + 1
    elseif dx < 0 then
        my_x = my_x - 1
    end

    if dy > 0 then
        my_y = my_y + 1
    elseif dy < 0 then
        my_y = my_y - 1
    end

    -- NPC�� ���ο� ��ġ�� �̵���ŵ�ϴ�.
    API_move_to(myid, my_x, my_y)
end