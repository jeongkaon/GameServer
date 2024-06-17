myid = 99999;

function set_uid(x)
   myid = x;
end

--어그로 검사하기
function event_player_move(player)
   player_x = API_get_x(player)
   player_y = API_get_y(player)
   my_x = API_get_x(myid)
   my_y = API_get_y(myid)


   -- x와 y 좌표만약 1이면 공격하는 메시지 보내기
   -- 이미 어그로 범위로 들어온애들임. 
   if math.abs(player_x - my_x) <= 1 and math.abs(player_y - my_y) <= 1 then
      API_SendMessage(myid, player, "ATTACK")
   end
  
end


function event_player_mov1e(player)
    player_x = API_get_x(player)
    player_y = API_get_y(player)
    my_x = API_get_x(myid)
    my_y = API_get_y(myid)

    -- 플레이어의 위치와 NPC 위치의 차이를 구하여 방향을 설정합니다.
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

    -- NPC를 새로운 위치로 이동시킵니다.
    API_move_to(myid, my_x, my_y)
end